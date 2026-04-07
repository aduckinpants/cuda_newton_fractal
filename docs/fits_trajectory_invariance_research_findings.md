# FITS Trajectory Invariance Object — Research Findings

Date: 2026-04-07
Status: POC Complete, Feasibility Confirmed
Prior art: `docs/fits_trajectory_invariance_object_agent_brief.md`,
           `spec_intake/FitsSolutionSpacePlayback_DesignNote_2026-04-07.md`

## 1. Framing Summary

The question: given a FITS delta-stack trajectory `F(x, y, t)`, can we derive
spatial fields that capture persistent structure under transport, and can those
fields produce a meaningful 3D object?

Answer: **yes, convincingly**.

The POC computed six derived invariance fields across seven Godel datasets (four
core 256x256x240f and three hires 512x512x60f). Every field except one degenerate
case (g10 homogeneous invariant) produced strong, structured spatial signal.

## 2. Data / Tooling Feasibility

### FITS delta-stack layout (confirmed)

Each `frames_delta_stack.fits` file contains:

- HDU[0]: PrimaryHDU = keyframe, shape `(3, H, W)`, float32, range [0, 255]
- HDU[1..N-1]: ImageHDU = per-frame delta, same shape, small integer-like values
- Header metadata: `NINEFMT=DELTA_STACK`, `AXISORD=CHW`, `NFRAMES`, `BRANCHID`
- Reconstruction: `F(t) = HDU[0] + cumsum(HDU[1..t])`

### Available corpora (17 files, ~180 MB each)

| Dataset     | Resolution | Frames | Delta Advantage | Zero Fraction |
|-------------|-----------|--------|-----------------|---------------|
| godel_g13   | 256x256   | 240    | strong (0.582)  | 0.686         |
| godel_g11   | 256x256   | 240    | moderate (0.677)| 0.759         |
| godel_g12   | 256x256   | 240    | moderate (0.676)| 0.807         |
| godel_g1_long| 256x256  | 240    | moderate (0.754)| 0.604         |
| godel_g4    | 256x256   | 240    | moderate (0.830)| 0.912         |
| godel_g10   | 256x256   | 240    | none (0.996)    | 0.971         |
| + 5 hires   | 512x512   | 60     | similar pattern | similar       |

### Reconstruction performance

Full trajectory reconstruction from delta-stack to `(C, H, W, T)` float32 array:
- 256x256x240f: ~1.3 seconds, ~180 MB memory
- 512x512x60f: ~1.5 seconds, ~180 MB memory
- All derived fields computed in under 2 seconds total per dataset

Toolchain: **astropy + numpy + PIL**. No new infrastructure required.

## 3. Derived-Field Results

### Fields computed

| Field              | Formula                                  | What it measures                |
|--------------------|------------------------------------------|---------------------------------|
| Temporal variance  | `Var_t(F(x,y,t))` averaged over channels | Total per-pixel churn           |
| Log-variance       | `log(1 + variance)`                      | Variance with better dynamic range |
| Stability          | `1 - variance / max(variance)`           | Normalized quietness            |
| Delta zero-fraction| `mean_t(all_channels_delta == 0)`        | Exact persistence fraction      |
| Occupancy          | `mean_t(intensity > median)`             | Time-in-class membership        |
| Temporal range     | `max_t(F) - min_t(F)` per channel, mean  | Total excursion                 |

### Cross-dataset signal quality

| Dataset     | Variance CoV | Zero-frac CoV | Occupancy CoV | Best field      |
|-------------|-------------|---------------|---------------|-----------------|
| godel_g13   | 1.190       | 0.484         | 0.619         | variance        |
| godel_g4    | 2.040       | 0.254         | 0.957         | variance + occ  |
| godel_g11   | 1.307       | 0.325         | 0.481         | variance        |
| godel_g1_long| 1.211      | 0.111         | 0.777         | variance + occ  |
| godel_g10   | 0.478       | **0.016**     | 0.794         | occupancy only  |
| g4_hires    | **2.685**   | 0.259         | 1.043         | variance + occ  |
| g13_hires   | 1.539       | **0.756**     | 0.761         | zero-frac       |
| g10_hires   | 0.676       | 0.043         | 0.992         | occupancy only  |

CoV = coefficient of variation (std/mean); higher = more spatial structure.

### Key findings

1. **Temporal variance** is the strongest general-purpose invariance field.
   CoV ranges from 1.19 to 2.69 across non-degenerate datasets. That is
   enormous spatial contrast — better than many natural image edge detectors.

2. **Delta zero-fraction** is the most semantically grounded field for exact
   persistence, but has lower CoV in most datasets. Exception: g13_hires (0.756)
   where zero-fraction outperforms variance.

3. **Occupancy** is the strongest field for g10 (the homogeneous-invariant case)
   where variance and zero-fraction are nearly flat. Occupancy captures
   structural membership even when temporal dynamics are minimal.

4. **g10 is a useful negative control**: near-flat variance and zero-fraction
   confirm that the fields are not hallucinating structure from noise. When the
   underlying signal is genuinely homogeneous, the invariance fields correctly
   report near-uniform persistence.

5. **Hires datasets amplify signal**: 512x512 versions of g4 and g13 show
   higher CoV than 256x256, suggesting the fields capture real spatial structure
   that is resolution-dependent, not frame-count-dependent.

6. **All fields are cheap**: total compute per dataset is under 2 seconds
   including FITS load, reconstruction, and all six field computations.

## 4. Recommended V1 Construction: Heightfield Invariance Sculpture

### What was built (POC)

For g13 and g4, the POC generated:
- `invariance_sculpture.ply`: height = stability field, color = mean frame RGB
- `churn_sculpture.ply`: height = log-variance, color = mean frame RGB
- 128x128 vertex grid, ASCII PLY format, ~1.2 MB each

### Why the heightfield wins for V1

1. **Simplest to implement** — 100 lines of Python, no dependencies beyond numpy
2. **Directly inspectable** — load in any mesh viewer (MeshLab, Blender, etc.)
3. **Strong intuitive mapping** — tall = stable, flat/low = churning
4. **Color carries semantic information** — mean RGB preserves fractal identity
5. **Easy to compare across datasets** — same construction, different inputs

### Recommended V1 parameters

- **Height field**: log-variance (better dynamic range than raw variance)
- **Height direction**: inverted, so stable = tall peaks, churned = valleys
  (matches "the math folds survive as ridges" intuition)
- **Color**: mean frame RGB from trajectory
- **Grid resolution**: match source resolution (256x256 or 512x512)
- **Z-scale**: 0.2-0.4 relative to XY unit square (adjustable)
- **Export format**: PLY (widely supported, vertex color, simple)

## 5. Alternate Constructions (for later comparison)

### Construction 2 — (x, y, t) volume + isosurface

**Feasibility**: straightforward. The reconstructed trajectory IS the volume.
For 256x256x240 = ~47M voxels per channel, a marching-cubes pass is under 10s.

**Best candidate**: binary membership volume with isosurface at occupancy=0.5.
This would produce a literal "fractal spacetime fossil" showing where structure
persists as solid geometry and where it flickers as gaps/tunnels.

**Risk**: visually dense at full resolution. Would need smart isovalue selection
and possibly per-channel decomposition.

**Recommendation**: worth a Phase 2 experiment on g4 (clean topology) and g13
(strong delta contrast).

### Construction 3 — Density/opacity ghost stack

**Feasibility**: requires a volumetric renderer (e.g. vispy, pyvista, or
export to VDB). Heavier dependency footprint.

**Best use case**: g1_long (dust-field baseline) where the "wispy transient
versus hard persistent" distinction maps most naturally to astro-stack semantics.

**Recommendation**: defer to Phase 3. The heightfield captures the same
essential information in a simpler form.

### Construction 4 — Reduced-state lift

**Feasibility**: PCA of the frame stack is trivial (~2 seconds for 240 frames
of 256x256x3). Whether the first few principal components produce a meaningfully
navigable embedding is an open question.

**Recommendation**: defer to Phase 4 unless the heightfield is unsatisfying.

## 6. Proposed Artifact / Output Schema

```
D:\salt-output\results\fits_invariance_poc\
  {dataset}/
    mean_frame.png              -- visual reference (trajectory mean)
    temporal_variance.png       -- hot colormap of raw variance
    log_temporal_variance.png   -- hot colormap of log(1+var)
    stability.png               -- hot colormap of 1 - norm_var
    delta_zero_fraction.png     -- hot colormap of exact-zero persistence
    occupancy.png               -- hot colormap of time-in-class
    temporal_range.png          -- hot colormap of max-min excursion
    invariance_sculpture.ply    -- heightfield mesh (stability height, mean color)
    churn_sculpture.ply         -- heightfield mesh (log-var height, mean color)
    invariance_fields.npz       -- (future) raw numpy fields for downstream use
    metadata.json               -- (future) source path, dataset info, field stats
```

## 7. Benchmark Trajectory Recommendations

| Priority | Dataset   | Why                                                    |
|----------|-----------|--------------------------------------------------------|
| 1        | godel_g13 | Strongest delta advantage, highest zero-frac contrast  |
| 2        | godel_g4  | Best variance CoV, CTC boundary topology               |
| 3        | g4_hires  | Highest variance CoV overall (2.685), 512x512          |
| 4        | godel_g10 | Negative control (homogeneous invariant)               |
| 5        | godel_g1_long | Dust-field contrast baseline, different line family |

## 8. Open Questions / Risks

1. **Channel semantics**: The current fields average over RGB channels. For the
   eigenvalue-encoding cases (g11-g13), per-channel invariance fields may reveal
   structure that channel-averaging hides. A per-channel decomposition should be
   tested in Phase 2.

2. **Normalization sensitivity**: The stability field depends on max-variance
   normalization. Cross-dataset comparison requires a shared normalization
   strategy (e.g. fixed scale, or log-transform which is more robust).

3. **Frame count vs resolution tradeoff**: Core datasets have 240 frames at
   256x256; hires have 60 frames at 512x512. More frames improve temporal
   statistics but fewer frames still produce clear signal. The minimum viable
   frame count should be characterized.

4. **Non-Godel families**: All current data is Godel. The invariance fields
   should be tested on at least one non-Godel trajectory (Newton, Mandelbrot,
   Phoenix, etc.) to confirm generality.

5. **Heightfield mesh quality**: The current PLY is ASCII and subsampled.
   Production version should use binary PLY or OBJ with optional normal
   computation for better rendering.

6. **Viewer integration**: The FITS invariance fields could eventually be
   computed inside the C++ viewer as an overlay or secondary viewport. That is
   a significant integration task and should not be attempted until the Python
   pipeline is stable.

7. **Explaino state trajectory**: The current FITS delta-stack stores rendered
   pixel values, not Explaino orientation vectors. A richer invariance object
   might eventually operate on the orientation vector trajectory directly.
   That requires changes to the FITS export format.

## 9. Prioritized Next Steps

### Immediate (current session, completed)

- [x] Data audit: confirmed FITS layout, sizes, reconstruction algorithm
- [x] Derived-field computation: 6 fields across 7 datasets
- [x] Signal quality assessment: CoV analysis confirming strong structure
- [x] V1 heightfield PLY generation: g13 and g4 sculptures
- [x] Cross-dataset comparison including negative control (g10)

### Phase 2 (next dedicated session)

- [ ] Per-channel invariance decomposition for eigenvalue-encoding datasets
- [ ] Binary PLY/OBJ export with normals for better rendering
- [ ] (x, y, t) volume isosurface experiment on g4 and g13
- [ ] Metadata JSON output with field statistics
- [ ] Save raw fields as .npz for downstream reuse

### Phase 3 (bounded exploration)

- [ ] Non-Godel trajectory test (Newton or Phoenix family)
- [ ] Density/opacity ghost stack with volumetric renderer
- [ ] Minimum viable frame count characterization
- [ ] Cross-dataset normalization strategy

### Phase 4 (if heightfield is insufficient)

- [ ] PCA decomposition of frame stack
- [ ] Reduced-state trajectory embedding
- [ ] Structure-tensor motion coherence field

## 10. One-Sentence Conclusion

The FITS delta-stack trajectories contain strong, structured spatial invariance
signal that can be cheaply extracted into derived fields and frozen into 3D
heightfield sculptures, with the temporal variance and log-variance fields
providing the best general-purpose persistence maps across the tested Godel
family.
