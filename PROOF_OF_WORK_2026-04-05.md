# Proof of Work: Explaino Seed Tween Fix + Fractal Analysis Toolkit

Date: 2026-04-05
Branch: working/salt-fractal-sweep-viewer
Latest commit: d2633b2 (warp fix) + pending toolkit additions

---

## 1. Bug Fix: Explaino Seed Tween Discontinuity

### Root Cause
`explaino_warp_start()` in `fractal_renderer.cu` applied `cx_rot(coord, rot)`
unconditionally. The rotation angle came from `LogisticAreaUToSeed()` (a chaotic
hash), so even with `warp_strength=0`, tiny seed changes spun the entire fractal.

### Fix (commit d2633b2)
- Early-return `coord` when `strength <= 0`
- Scale rotation by `strength`: `float rot = s * (a0 * 2.0f - 1.0f) * PI;`

### Diagnostic Proof
`test_seed_tween_continuity.cpp` confirms host model smoothness:
- seed 5.000 -> 5.001: root delta = 0.000005 (proportional)
- seed 5.999 -> 6.001 (boundary crossing): root delta = 0.000004

### Three Wrong Fixes (rejected)
1. LogisticAreaUToSeed as tween param -- chaotic hash, not monotone
2. Raw linear driftFrac -- too abrupt at boundaries
3. ExplainoWedgeTween only -- correct math but wrong layer; actual bug was GPU-side

### Key Lesson
"When visual output is jumpy, bisect: prove the host model is smooth with a
numeric continuity test. If model is smooth, the bug is in the renderer/GPU."

---

## 2. Captured Finding

Path: `D:\salt-fractal\cuda_newton_fractal_clone\findings\manual_capture\2026-04-05\220722_840__explaino_dual\`

Contents:
| File          | Size     |
|---------------|----------|
| state.json    | 1,190 B  |
| finding.json  | 564 B    |
| finding.md    | 438 B    |
| frame.png     | 249,908 B|
| analysis.json | 5,084 B  |

State snapshot:
- Fractal: explaino_dual
- Seed A=0.712, Seed B=8.74, Mix=0.5, Warp=0
- Center: (-0.677, 0.002), Zoom: 21.1 (log2=4.4)
- Coloring: joy_basins
- Polynomial: z^4 + 1.086z^3 - 0.571z^2 + 0.137z + 1.565

---

## 3. Analysis Results (finding_analyzer.py)

### Roots (4 roots, 2 conjugate pairs)
| Root | Re       | Im       |
|------|----------|----------|
| 0    | -1.2016  | -0.4418  |
| 1    | -1.2016  | +0.4418  |
| 2    | +0.6584  | +0.7221  |
| 3    | +0.6584  | -0.7221  |

### Root Geometry
- Centroid: (-0.272, 0.0)
- Mean radius from centroid: 1.103
- Radius spread: 0.148 (left pair slightly closer to centroid)
- Real-axis symmetry: YES (2 conjugate pairs)

### Basin Structure
| Basin | Root              | Fraction | Mean Iters |
|-------|-------------------|----------|------------|
| 0     | -1.20 - 0.44i     | 29.1%    | 17.2       |
| 1     | -1.20 + 0.44i     | 28.9%    | 17.2       |
| 2     | +0.66 + 0.72i     | 1.2%     | 21.4       |
| 3     | +0.66 - 0.72i     | 1.2%     | 21.1       |
| unconv| (dark lobes)      | 39.7%    | --         |

Left pair dominates (~58% combined) vs right pair (~2.4%). The 39.7%
unconverged fraction corresponds to the dark lobe regions visible in frame.png.

### Frame Metrics
- Mean luma: 114, Std: 33
- Edge energy: 2.63 (moderate fractal detail)
- Channel means: R=152, G=96, B=108 (warm salmon tones)
- Dominant colors: salmon/rose (29%), lavender (27%), dark near-black (7.5%)

### Symmetry
- Horizontal reflection: 0.890
- Vertical reflection: 0.899
- Bilateral: 0.894

High bilateral symmetry is consistent with the conjugate-pair root structure
and zero warp strength.

---

## 4. New Tooling Delivered

### finding_analyzer.py (~420 lines)
`tools/reality_toolkit/fractal_explorer/finding_analyzer.py`

Capabilities:
- Polynomial root-finding via Newton iteration on grid of start points
- Basin classification and basin-map computation
- Root geometry analysis (centroid, radii, conjugate pairs, symmetry)
- Frame metrics (luma, edge energy, channel means)
- Color clustering (4-bit quantization, top-8 clusters)
- Bilateral symmetry measurement (horizontal + vertical reflection)
- JSON + text report output

### run_fractal_finding_analyzer.py (CLI runner)
`tools/reality_toolkit/scripts/run_fractal_finding_analyzer.py`

Usage:
```
python tools/reality_toolkit/scripts/run_fractal_finding_analyzer.py <finding_dir>
python tools/reality_toolkit/scripts/run_fractal_finding_analyzer.py --finding-id 220722_840__explaino_dual --date 2026-04-05
```

### test_fractal_finding_analyzer.py (8 tests, all passing)
`tests/test_fractal_finding_analyzer.py`

Tests: poly_eval, newton convergence, root-finding (z^4-1 and explaino_dual),
basin classification, basin map (z^3-1), root geometry, full roundtrip.

### UI: Copy Path + Open Folder buttons
`ui_app/src/main.cpp` -- after Capture Finding, the status bar now shows:
- **Copy Path** button: copies finding directory to clipboard
- **Open Folder** button: opens Windows Explorer at the finding directory

---

## 5. Gap Analysis: Identified + Partially Filled

| Gap | Status |
|-----|--------|
| No analysis tooling for captured findings | FILLED (finding_analyzer.py) |
| Finding path not easily copyable/openable | FILLED (ImGui buttons) |
| No convergence rate heatmap export | Open |
| No basin boundary extraction / Hausdorff distance | Open |
| No polynomial continuation / path-following | Open |
| No bridge between nine's cross-tool atlas and findings | Open |
| No seed-sweep + analysis pipeline (batch mode) | Partial (seed_sweep.py exists, needs analyzer integration) |

---

## 6. Files Changed (uncommitted)

### New files:
- `tools/reality_toolkit/fractal_explorer/finding_analyzer.py`
- `tools/reality_toolkit/scripts/run_fractal_finding_analyzer.py`
- `tests/test_fractal_finding_analyzer.py`

### Modified files:
- `tools/reality_toolkit/fractal_explorer/__init__.py` (exports)
- `ui_app/src/main.cpp` (Copy Path + Open Folder buttons)

### Memory files created:
- `/memories/fractal_debugging_lessons.md`
- `/memories/repo/warp_rotation_must_be_gated.md`
- `/memories/repo/wedge_tween_vs_logistic_hash.md`
- `/memories/repo/seed_continuity_test_pattern.md`

### Finding artifacts:
- `D:\salt-fractal\...\220722_840__explaino_dual\analysis.json` (generated by analyzer)
