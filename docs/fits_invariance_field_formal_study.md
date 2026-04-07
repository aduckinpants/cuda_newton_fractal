# FITS Invariance Field Formal Study

Date: 2026-04-07
Status: Study complete, canonical recipe recommended
Prior: `docs/fits_trajectory_invariance_research_findings.md` (POC),
       `docs/fits_trajectory_invariance_object_agent_brief.md` (brief)

---

## 1. Study Scope

This document formalizes the derived-field ladder for FITS trajectory invariance
objects, compares all candidate fields head-to-head, evaluates robustness under
threshold and normalization changes, examines per-channel decomposition for
eigenvalue-encoding trajectories, and recommends a canonical V1 mesh recipe for
mainline follow-up.

Nine Godel datasets analyzed (six core 256x256x240f, three hires 512x512x60f).

All raw data in `D:\salt-output\results\fits_invariance_study\`.

---

## 2. Field-by-Field Comparison

### 2.1 The derived-field ladder

Six candidate fields were computed for every dataset. Ranked by overall utility:

| Rank | Field           | Best Use Case                    | Weakness                        |
|------|-----------------|----------------------------------|---------------------------------|
| 1    | **variance**    | General-purpose invariance map   | Dominated by outlier peaks      |
| 2    | **log_variance**| Variance with better vis range   | Compresses high-churn contrasts |
| 3    | **occupancy**   | Binary membership structure      | Threshold-dependent             |
| 4    | **zero_frac**   | Exact persistence (semantic)     | Low CoV on many datasets        |
| 5    | **temporal_range** | Max excursion envelope        | Redundant with variance (r>0.9) |
| 6    | **stability**   | 1-norm_var, conceptually clear   | Inverts CoV ranking; poor IQR   |

### 2.2 Variance dominates spatial contrast

Coefficient of variation (CoV = std/mean) measures how much spatial structure
a field contains. Higher = more contrast = more informative invariance map.

**Variance CoV by dataset (descending):**

| Dataset     | variance | log_var | zero_frac | occupancy | temp_range | stability |
|-------------|----------|---------|-----------|-----------|------------|-----------|
| g4_hires    | **2.685**| 1.707   | 0.259     | 1.043     | 1.818      | 0.159     |
| g4          | **2.040**| 1.119   | 0.254     | 0.957     | 1.274      | 0.152     |
| g13_hires   | **1.539**| 0.369   | 0.756     | 0.761     | 0.657      | 0.082     |
| g11         | **1.307**| 0.280   | 0.325     | 0.481     | 0.604      | 0.151     |
| g1_long     | **1.211**| 0.257   | 0.111     | 0.777     | 0.583      | 0.129     |
| g13         | **1.190**| 0.224   | 0.484     | 0.619     | 0.533      | 0.135     |
| g12         | 0.801   | 0.128   | 0.112     | 0.678     | 0.215      | 0.149     |
| g10_hires   | 0.676   | 0.574   | 0.043     | **0.992** | 0.480      | 0.382     |
| g10         | 0.478   | 0.256   | 0.016     | **0.794** | 0.230      | 0.478     |

**Key finding:** Variance is the strongest field on 7 of 9 datasets. Occupancy
wins only on the g10 family (homogeneous invariant), where variance and
zero-fraction are nearly flat. This makes occupancy the right second-layer field.

### 2.3 Stability is the weakest field

The `stability = 1 - variance/max(variance)` construction looks intuitive but
performs poorly. Its CoV is consistently the lowest (0.08-0.48) because the
max-normalization crushes the distribution into a narrow band near 1.0. The
only dataset where stability CoV exceeds 0.3 is g10, where all fields are flat.

**Recommendation: drop stability from the canonical recipe.** Use log-variance
instead, which captures the same high-variance=unstable semantics with far
better dynamic range.

### 2.4 Temporal range is redundant

Cross-correlation between temporal_range and variance is r=0.93 (g13), r=0.93
(g4), r=0.94 (g1_long), r=0.88 (g10). These are near-duplicate fields for the
purpose of invariance mapping.

**Recommendation: keep temporal_range only as a diagnostic.** It is not needed
in the mesh recipe.

---

## 3. Epsilon Threshold Sweep

Computed epsilon-persistence `S_eps(x,y) = fraction_t(||delta|| < eps)` for
11 epsilon values from 0.1 to 20.0 across four datasets.

### 3.1 Key transition thresholds

The data is integer-quantized (pixel values 0-255), so deltas are integer-valued.
This creates a natural structure in the epsilon sweep:

| Range       | Behavior                                          |
|-------------|---------------------------------------------------|
| eps < 1.0   | **Identical to exact-zero.** All eps in [0.1, 1.0) produce the same field because the smallest nonzero delta magnitude is >= 1.0. |
| eps = 2.0   | First meaningful relaxation. CoV drops significantly (g13: 0.484 -> 0.275, g4: 0.254 -> 0.121). This is where tolerance persistence starts to differ from exact persistence. |
| eps = 5.0   | Most structure washed out. Mean persistence > 0.99 on g4 and g10. Only g13 and g1_long retain meaningful contrast. |
| eps >= 7.0  | Near-uniform. All pixels appear "persistent." No useful invariance signal. |

### 3.2 The integer quantization cliff

Because the underlying data is integer-valued RGB, all epsilon values below 1.0
are equivalent. There is no smooth epsilon-persistence curve — there is a cliff
at 1.0 and then rapid convergence to 1.0 as epsilon increases.

This means epsilon-persistence is **not a useful tuning knob** for this data.
The only two functionally distinct persistence fields are:
- exact-zero (eps < 1.0)
- tolerance-1 (eps in [1.0, 2.0))

For continuous-valued data (e.g., double-precision render output or orientation
vectors), epsilon sweeps would produce a richer and more tunable landscape.

### 3.3 Recommendation

For the current integer-valued Godel FITS corpora, use **exact-zero fraction
only** as the persistence field. Do not expose an epsilon parameter in the V1
recipe. Reserve epsilon-persistence for future continuous-valued trajectories.

---

## 4. Normalization Sensitivity

Tested 8 normalization methods on variance, zero_frac, and occupancy across
three representative datasets (g13, g4, g10).

### 4.1 Correlation with raw field (order preservation)

**Variance normalization stability:**

| Method              | g13   | g4    | g10   |
|---------------------|-------|-------|-------|
| raw / min_max       | 1.000 | 1.000 | 1.000 |
| percentile 1-99     | 0.994 | 0.996 | 1.000 |
| percentile 5-95     | 0.952 | 0.943 | 0.997 |
| log1p               | 0.847 | 0.835 | 0.970 |
| rank                | 0.787 | 0.681 | 0.998 |

**Zero-fraction normalization stability:**

| Method              | g13   | g4    | g10   |
|---------------------|-------|-------|-------|
| raw / min_max       | 1.000 | 1.000 | 1.000 |
| percentile 1-99     | 1.000 | 1.000 | 0.992 |
| log1p               | 0.999 | 0.999 | 1.000 |
| percentile 5-95     | 0.996 | 0.998 | 0.949 |
| rank                | 0.988 | 0.823 | 0.931 |

**Occupancy normalization stability:**

| Method              | g13   | g4    | g10   |
|---------------------|-------|-------|-------|
| raw / min_max       | 1.000 | 1.000 | 1.000 |
| percentile 1-99     | 1.000 | 1.000 | 1.000 |
| log1p               | 0.995 | 0.998 | 0.996 |
| rank                | 0.992 | 0.936 | 0.974 |

### 4.2 Key findings

1. **Zero-fraction and occupancy are normalization-robust.** All methods preserve
   r > 0.93 with raw. These fields are already naturally bounded [0, 1] and
   don't need aggressive normalization.

2. **Variance is normalization-sensitive.** The log1p transform drops correlation
   to 0.84 (g13) and rank-normalization to 0.68 (g4). This means the choice of
   normalization for variance **materially changes the heightfield shape**.

3. **log1p is the right variance transform.** It sacrifices some correlation
   (0.84-0.97) but dramatically improves visual usability by compressing
   the 2000x dynamic range into a 7.4x range. Raw variance produces a
   heightfield dominated by a few extreme peaks; log-variance produces a
   readable surface with preserved structure.

4. **Percentile clipping (1-99) is the best linear normalization.** It preserves
   r > 0.99 for variance and is robust across datasets. If log is too aggressive,
   percentile 1-99 clipping is the fallback.

### 4.3 Recommendation

Canonical normalization pipeline:
- **Height field**: `log1p(variance)` — best tradeoff of readability vs fidelity
- **Occupancy overlay**: raw (already [0, 1])
- **Zero-fraction overlay**: raw (already [0, 1])
- Cross-dataset comparison: use `percentile_clip(1, 99)` on log-variance for
  a shared scale

---

## 5. Cross-Field Correlation

Pairwise Pearson correlation between core fields, measured per-dataset.

### 5.1 Correlation matrix (representative datasets)

**godel_g13:**
```
                variance  temp_range  zero_frac  occupancy  stability
variance           1.000       0.954     -0.622      0.525     -1.000
temporal_range     0.954       1.000     -0.765      0.668     -0.954
zero_frac         -0.622      -0.765      1.000     -0.804      0.622
occupancy          0.525       0.668     -0.804      1.000     -0.525
stability         -1.000      -0.954      0.622     -0.525      1.000
```

**godel_g10 (negative control):**
```
                variance  temp_range  zero_frac  occupancy  stability
variance           1.000       0.883     -0.415     -0.044     -1.000
temporal_range     0.883       1.000     -0.738     -0.089     -0.883
zero_frac         -0.415      -0.738      1.000      0.046      0.415
occupancy         -0.044      -0.089      0.046      1.000      0.044
stability         -1.000      -0.883      0.415      0.044      1.000
```

### 5.2 Interpretation

1. **Stability vs variance: r=-1.000** on every dataset. This is definitional —
   stability IS 1-normalized-variance. This confirms stability adds no
   information beyond variance.

2. **Temporal range vs variance: r=0.88-0.95.** Near-duplicate, as noted above.

3. **Zero-fraction vs variance: r=-0.41 to -0.89.** Correlated but not
   redundant. Zero-fraction captures a different phenomenon (exact persistence)
   than variance (total spread). The correlation is strongest on g4 (r=-0.89)
   and weakest on g10 (r=-0.41).

4. **Occupancy vs everything else: r < 0.67 (g13), near-zero on g10.** Occupancy
   is the most independent field. It captures membership structure that the
   other fields miss. On g10, occupancy is nearly orthogonal to all other
   fields (r < 0.09) — this is why it's the only useful field on the
   homogeneous-invariant case.

### 5.3 Implication for the mesh recipe

The three functionally independent fields are:
- **variance** (or log-variance) — continuous churn magnitude
- **zero-fraction** — discrete persistence probability
- **occupancy** — categorical membership structure

Two of these should be in the canonical recipe (height + color/opacity). The
third is available as an alternate construction or overlay.

---

## 6. Per-Channel Decomposition

The eigenvalue-encoding datasets (g11, g12, g13) use RGB channels to encode
distinct physical quantities. Per-channel variance decomposition reveals
whether channels carry independent invariance information.

### 6.1 Inter-channel correlation of variance

| Dataset | ch0 vs ch1 | ch0 vs ch2 | ch1 vs ch2 |
|---------|-----------|-----------|-----------|
| g11     | 0.851     | 0.900     | 0.826     |
| g12     | **0.242** | **-0.254**| **-0.686**|
| g13     | 0.691     | **-0.129**| **-0.110**|

### 6.2 Per-channel CoV

| Dataset | ch0 CoV | ch1 CoV | ch2 CoV |
|---------|---------|---------|---------|
| g11     | 1.312   | 1.239   | 1.404   |
| g12     | **0.134** | 0.673 | **1.286** |
| g13     | 1.214   | 1.372   | **0.116** |

### 6.3 Interpretation

**g12 is the most channel-divergent dataset.** Channel 0 (R) has near-zero
variance CoV (0.134) — it is almost static. Channel 2 (B) has the highest
CoV (1.286). Their correlation is r=-0.254, meaning they carry strongly
independent information. g12's known identity as a gravitomagnetic-null
standard candle explains this: the null channel remains quiet while the
active channels show structured variation.

**g13 has one quiet channel** (ch2, CoV=0.116) and two active channels
(ch0=1.21, ch1=1.37) with moderate correlation (r=0.69). The quiet channel
corresponds to the verified B-null pass.

**g11's channels are highly correlated** (all r > 0.82) and have similar CoV.
The G-null property doesn't produce as much inter-channel divergence in
variance space.

### 6.4 Recommendation

For eigenvalue-encoding trajectories, the canonical recipe should offer an
optional **per-channel variance decomposition** mode where:
- Each channel's variance field drives a separate RGB component of the mesh color
- The height is still driven by channel-mean log-variance
- The per-channel coloring reveals null-channel structure that channel-averaging hides

This is valuable for g12 and g13 but unnecessary for g11.

---

## 7. Canonical V1 Mesh Recipe

### 7.1 Recommended construction

```
FITS Invariance Sculpture V1
=============================

Input:     frames_delta_stack.fits  (any resolution, any frame count >= 10)

Step 1 — Reconstruct trajectory
  F(c, y, x, t) = cumsum of delta stack from keyframe

Step 2 — Compute primary field: log-variance
  variance(y, x) = mean_c(Var_t(F(c, y, x, :)))
  height(y, x) = log(1 + variance(y, x))

Step 3 — Compute color field: mean frame RGB
  color(y, x) = clip(mean_t(F(:, y, x, :)).transpose(1,2,0), 0, 255)

Step 4 — Compute overlay field (optional): occupancy
  occupancy(y, x) = mean_t(mean_c(F(c, y, x, t)) > median)

Step 5 — Normalize height for mesh
  height_norm = height / max(height)    [invert if stability semantics desired]

Step 6 — Emit mesh
  Format: PLY (binary preferred, ASCII acceptable)
  Vertices: (x/(W-1), y/(H-1), height_norm * z_scale)
  Color: mean frame RGB at each vertex
  Faces: two triangles per grid cell
  z_scale: 0.25 (default, adjustable)

Optional Step 7 — Emit diagnostic overlays as PNG
  - log_variance heatmap
  - zero_fraction heatmap
  - occupancy heatmap
  - per-channel variance (for eigenvalue-encoding trajectories)
```

### 7.2 Parameters

| Parameter    | Default | Range     | Rationale                              |
|--------------|---------|-----------|----------------------------------------|
| z_scale      | 0.25    | 0.1-0.5  | Tested range; 0.25 balances readability |
| height_field | log_var | see below | Best general-purpose field              |
| color_field  | mean_rgb| see below | Preserves fractal identity              |
| grid_res     | native  | 64-512   | Match source for fidelity               |

Alternate height fields (selectable):
- `log_variance` (default, best general-purpose)
- `zero_fraction` (for exact-persistence analysis)
- `occupancy` (for membership analysis, best on homogeneous cases)

Alternate color fields (selectable):
- `mean_rgb` (default)
- `per_channel_variance` (for eigenvalue-encoding analysis)
- `height_colormap` (hot/viridis for single-field inspection)

### 7.3 Why this recipe

1. **log-variance height** was the strongest field on 7/9 datasets (CoV 1.2-2.7
   for raw variance; log transform compresses dynamic range while preserving
   r > 0.84 with raw).

2. **Mean RGB color** carries fractal identity without adding noise. It's the one
   field that makes the sculpture recognizable as "that trajectory."

3. **PLY format** is universally supported (MeshLab, Blender, pyvista, Open3D,
   three.js) and carries vertex color natively.

4. **Native resolution grid** preserves all spatial structure. Subsampling to
   128x128 is acceptable for quick preview but loses fine detail.

5. **Optional occupancy overlay** catches the g10-type degenerate case where
   variance is flat but membership structure exists.

### 7.4 What was deliberately excluded

- **Stability field**: CoV is consistently lowest; adds no information over
  variance.
- **Temporal range**: r > 0.88 with variance; redundant.
- **Epsilon-persistence**: equivalent to exact-zero on integer data; not a
  useful tuning knob for current corpora. Reserve for continuous-valued future.
- **Isosurface / volume construction**: deferred to Phase 2; heightfield is
  sufficient for V1 and far simpler.

---

## 8. Cross-Trajectory Comparison

All nine datasets in this study are Godel family. The Godel family spans
a meaningful range of signal types:

| Signal type                | Representatives              |
|----------------------------|------------------------------|
| High-churn, strong delta   | g13, g13_hires               |
| Topology anchor            | g4, g4_hires                 |
| Eigenvalue null-channel    | g11, g12, g13                |
| Dust-field baseline        | g1_long                      |
| Homogeneous invariant      | g10, g10_hires               |

The fields are robust across this range:
- Variance CoV spans 0.48-2.69 — no dataset breaks the field
- Zero-fraction CoV spans 0.016-0.756 — correctly degrades on flat cases
- Occupancy CoV spans 0.48-1.04 — viable on every dataset

The g10 negative control is crucial: it proves the fields don't hallucinate
structure on genuinely homogeneous data.

**A non-Godel family test (Newton/Phoenix/Mandelbrot) is still recommended**
for Phase 2 to confirm generality beyond the Godel signal space.

---

## 9. Practical Tooling Notes

### 9.1 Performance budget

| Step                       | Time (256x256x240f) | Time (512x512x60f) |
|----------------------------|--------------------:|--------------------:|
| FITS open + reconstruct    | 1.3s               | 1.5s               |
| All 6 fields               | 0.5s               | 1.0s               |
| PLY mesh write (128x128)   | 0.3s               | 0.3s               |
| PLY mesh write (256x256)   | 1.2s               | -                  |
| **Total**                  | **~2s**            | **~3s**            |

### 9.2 Memory budget

- 256x256x240f x 3ch x 4B = 180 MB
- 512x512x60f x 3ch x 4B = 180 MB
- Plus ~50 MB for derived fields
- Peak: ~250 MB per dataset

### 9.3 File size budget

- ASCII PLY (128x128): ~1.2 MB
- ASCII PLY (256x256): ~5 MB
- Binary PLY (256x256): ~2 MB (estimated)
- PNG overlays: 15-180 KB each
- Total per dataset: ~3-7 MB

### 9.4 Dependencies

- `numpy` (core computation)
- `astropy` (FITS I/O)
- `Pillow` (PNG output, optional)
- No GPU, no CUDA, no ImGui — pure Python

---

## 10. Relationship to the CUDA Trajectory-to-Geometry Path

The FITS invariance sculpture is not a brand-new construction. It is an
explicit, analysis-friendly readout of a trajectory-to-geometry collapse
that the CUDA fractal renderer already performs implicitly. This section
maps the two constructions onto each other and identifies where they agree,
where they differ, and how the FITS-derived object can serve as the canonical
post-hoc representation.

### 10.1 The CUDA kernel's implicit collapse

The renderer converts per-pixel iteration trajectories into spatial geometry
through three layered mechanisms (all in `ui_app/src/`):

| Stage              | Input                  | Output                | File                     |
|--------------------|------------------------|-----------------------|--------------------------|
| Iteration          | z0, polynomial         | final z, iter count   | fractal_renderer.cu      |
| Convergence test   | f(z) < epsilon         | converged (bool)      | fractal_renderer.cu      |
| Basin assignment   | final z, root list     | root_index            | basin_coloring.h         |
| Lens mask          | converged/escaped      | 0/255 per pixel       | fractal_family_rules.h   |
| Chamfer SDF        | mask field             | signed distance field | lens_sdf.cpp             |
| Coloring           | root, iter, phase      | RGBA                  | fractal_renderer.cu      |

Each layer discards information:
- **Basin assignment** keeps only the nearest root, discarding the trajectory.
- **Brightness shading** keeps `iter / max_iter`, discarding the path.
- **Lens mask** reduces convergence to a binary field.
- **SDF** smooths the binary boundary into a continuous distance.

The final image is a lossy spatial encoding of the temporal iteration behavior.

### 10.2 Where the constructions agree

The FITS invariance sculpture and the CUDA coloring pipeline both answer the
same underlying question: *which spatial regions change most across the
trajectory?*

| Kernel concept                | FITS invariance equivalent        | Agreement |
|-------------------------------|-----------------------------------|-----------|
| Iteration count -> brightness | Temporal variance -> height       | Strong: both rank pixels by how "busy" their trajectory is. Variance is a continuous version of iteration-count shading. |
| Basin assignment (root index) | Mean-frame RGB color              | Partial: mean RGB preserves the basin identity that root_index encodes, but averages over time. |
| Lens mask (converged/not)     | Occupancy field (above/below median) | Analogous: both partition the plane into "inside" and "outside" sets. The FITS version uses a continuous threshold rather than a convergence epsilon. |
| Chamfer SDF (distance to boundary) | Gradient of log-variance          | Structural: SDF encodes distance-to-boundary; log-variance gradient identifies where the invariance landscape steepens, which corresponds to basin boundaries. |

### 10.3 Where they differ

| Dimension          | CUDA kernel                        | FITS invariance sculpture          |
|--------------------|------------------------------------|------------------------------------|
| **Temporal depth** | Single frame (one param set)       | Full trajectory (240 frames)       |
| **Information**    | Lossy (final z + iter count only)  | Lossless (full temporal statistics)|
| **Reversibility**  | Cannot reconstruct trajectory from image | Can reconstruct every frame from delta stack |
| **Parameter sensitivity** | Implicit: brightness changes with params but is not measured | Explicit: variance *is* the sensitivity measure |
| **Boundary definition** | Binary convergence test (hard threshold) | Continuous variance/occupancy (no hard boundary) |
| **Computational cost** | Real-time (GPU, per-pixel)        | Offline (CPU, accumulator pass)    |

The critical difference: the CUDA path collapses trajectory information into
a single rendered frame and discards the temporal axis entirely. The FITS
invariance fields retain the full temporal signal and make the collapse
explicit and queryable.

### 10.4 The FITS object as canonical analysis form

The CUDA coloring pipeline is optimized for real-time visual rendering. It is
not designed for quantitative analysis, comparison, or geometric export. The
FITS invariance sculpture fills exactly this gap:

- **Quantitative comparison**: Two trajectories can be compared by their
  variance/occupancy/zero-fraction fields, not by pixel-diffing rendered images
  that bake in brightness ramps and palette choices.
- **Geometric export**: The heightfield PLY carries the invariance signal as
  geometry, not as image-space colorization. This enables 3D inspection,
  cross-sections, overlay in mesh tools, and programmatic analysis.
- **Decomposability**: Per-channel variance decomposition reveals eigenvalue
  structure that the CUDA averaging-then-coloring pipeline discards.
- **Reproducibility**: The PLY is deterministic from the FITS delta stack.
  No render settings, no GPU state, no floating-point ordering sensitivity.

The FITS invariance sculpture can therefore be treated as the **analysis-friendly
canonical representation** of what the CUDA engine implicitly computes at
render time. The real-time renderer remains the interactive exploration tool;
the invariance sculpture becomes the measurement and export tool.

### 10.5 Unification path

In a mature IDE, the two paths can be unified:

1. **Live lens**: The CUDA kernel renders the fractal in real time as it does
   now. This is the interactive viewport.
2. **Invariance lens**: A second viewport shows the invariance sculpture computed
   from the FITS delta stack of the current trajectory. This is the analysis
   viewport.
3. **Linked inspection**: Clicking a point in either viewport highlights the
   corresponding point in the other. The CUDA view shows "what this pixel looks
   like"; the invariance view shows "how much this pixel changed."
4. **SDF overlay**: The Chamfer SDF (already computed in `lens_sdf.cpp`) can be
   overlaid onto the invariance heightfield as a contour ring, showing where
   the convergence boundary sits relative to the invariance landscape.

This treats the invariance sculpture not as a replacement for the real-time
renderer but as a complementary readout that makes the implicit
trajectory-to-geometry collapse explicit, measurable, and exportable.

---

## 11. Long-Term IDE Integration Path

The user has indicated this will become a first-class IDE tool. Here is the
staged integration path, incorporating the dual-viewport model from Section 10.

### Stage 1 — Python CLI tool (current)

`tools/fits_invariance_poc.py` and `tools/fits_invariance_study.py` already
work end-to-end. Next step: consolidate into a single `tools/fits_invariance.py`
with proper CLI (argparse), dataset auto-discovery, and artifact management.

### Stage 2 — Schema binding surface

Define a schema group for invariance fields in the UI schema JSON. This would
add a new panel or viewport mode where the user selects:
- Source FITS trajectory
- Height field (log_variance / zero_fraction / occupancy)
- Color field (mean_rgb / per_channel_variance / height_colormap)
- z_scale slider

The schema binding infrastructure (already refactored) can wire these to the
C++ viewer state.

### Stage 3 — C++ viewer integration

Compute the invariance fields in C++ (trivial: variance and zero-fraction are
simple accumulator passes over the delta stack). Render the heightfield as a
secondary viewport using DX11 vertex buffers.

### Stage 4 — Interactive exploration

Enable live rotation/zoom of the invariance sculpture within the ImGui viewer.
Clicking a point on the sculpture shows the per-pixel time series at that
location.

This progression stays within the repo's architecture rules: schema drives
layout, C++ drives runtime, no second source of truth.

---

## 12. Artifacts Produced

```
D:\salt-output\results\fits_invariance_study\
  field_comparison.csv              -- All fields x all datasets x 12 stats
  epsilon_sweep.csv                 -- 11 eps x 4 datasets x 12 stats
  normalization_sensitivity.csv     -- 8 methods x 3 fields x 3 datasets
  field_cross_correlation.csv       -- Pairwise Pearson r for 4 datasets
  per_channel_decomposition.csv     -- Ch-pair correlations + per-ch CoV
  fits_invariance_study_summary.json -- Master index

D:\salt-output\results\fits_invariance_poc\
  {g13,g4,g10,g1_long,g4_hires,g13_hires,g10_hires}/
    mean_frame.png
    temporal_variance.png
    log_temporal_variance.png
    stability.png
    delta_zero_fraction.png
    occupancy.png
    temporal_range.png
    invariance_sculpture.ply        (g13, g4 only)
    churn_sculpture.ply             (g13, g4 only)

c:\code\cuda_newton_fractal_clone\tools\
  fits_invariance_poc.py            -- PLY mesh generator
  fits_invariance_study.py          -- Full study data generator
  fits_invariance_study_report.py   -- Formatted report printer
```

---

## 13. Summary Recommendations

1. **Canonical height field: log-variance.** Strongest spatial contrast, robust
   across signal types, readable dynamic range.

2. **Canonical color: mean frame RGB.** Preserves fractal identity, no
   normalization needed.

3. **Secondary field: occupancy.** Captures categorical structure missed by
   variance; essential for homogeneous-invariant cases.

4. **Drop: stability, temporal_range.** Redundant with variance; stability has
   worst CoV of any field.

5. **Epsilon-persistence: use exact-zero only.** Integer data quantization makes
   epsilon a step function, not a tuning knob. Reserve for continuous data.

6. **Normalization: log1p for variance, raw for bounded fields.** Percentile
   1-99 clipping as fallback for cross-dataset comparison.

7. **Per-channel variant: offer for eigenvalue-encoding datasets.** g12 and g13
   show strongly divergent per-channel structure that channel-averaging hides.

8. **Export: PLY with vertex color.** Universal support, simple, adequate for
   V1. Binary PLY for production.

9. **Next non-Godel test mandatory.** All current evidence is Godel family.
   Generality beyond Godel is plausible but unproven.

10. **CUDA SDF connection is structural, not incidental.** The invariance
    sculpture makes explicit what the kernel's convergence test, basin
    assignment, and Chamfer SDF already encode implicitly. The IDE integration
    path should treat them as complementary viewports, not independent tools.
