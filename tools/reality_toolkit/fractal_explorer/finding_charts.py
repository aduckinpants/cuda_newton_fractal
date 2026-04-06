"""Generate visual charts and CSV exports for a FindingAnalysis.

Produces into an output directory:
  - root_constellation.png  -- roots in complex plane, sized by basin fraction
  - basin_pie.png           -- basin share pie chart
  - color_palette.png       -- dominant color swatches
  - convergence_heatmap.png -- iteration-count heatmap over viewport
  - symmetry_bars.png       -- symmetry score bar chart
  - frame_annotated.png     -- original frame with root markers overlaid
  - roots.csv               -- root positions + basin fractions
  - basins.csv              -- basin counts and iteration stats
  - frame_metrics.csv       -- single-row frame quality metrics
  - summary.md              -- human-readable one-page summary with inline images

Requires: matplotlib, PIL (Pillow) -- both optional at import time so the
core finding_analyzer.py stays dependency-free.
"""
from __future__ import annotations

import csv
import math
import os
from dataclasses import asdict
from pathlib import Path
from typing import List, Optional, Tuple

from .finding_analyzer import (
    Complex,
    FindingAnalysis,
    analyze_finding,
    compute_basin_map,
    find_roots_numerically,
    format_report,
    newton_iterate,
    poly_eval,
)

# ---------------------------------------------------------------------------
# Color helpers
# ---------------------------------------------------------------------------

# Per-basin palette: visually distinct, accessible, matches the "joy" theme.
BASIN_COLORS = [
    "#e6194B",  # red
    "#3cb44b",  # green
    "#4363d8",  # blue
    "#f58231",  # orange
    "#911eb4",  # purple
    "#42d4f4",  # cyan
    "#f032e6",  # magenta
    "#bfef45",  # lime
]
UNCONVERGED_COLOR = "#1a1a2e"

def _rgb_tuple(hex_color: str) -> Tuple[float, float, float]:
    h = hex_color.lstrip("#")
    return tuple(int(h[i:i+2], 16) / 255.0 for i in (0, 2, 4))


# ---------------------------------------------------------------------------
# Chart generators
# ---------------------------------------------------------------------------

def chart_root_constellation(analysis: FindingAnalysis, out_path: Path) -> None:
    """Plot roots in the complex plane, sized by basin fraction."""
    import matplotlib.pyplot as plt

    fig, ax = plt.subplots(figsize=(6, 6))
    ax.set_aspect("equal")
    ax.set_title("Root Constellation", fontsize=14, fontweight="bold")
    ax.set_xlabel("Re(z)")
    ax.set_ylabel("Im(z)")
    ax.axhline(0, color="#888", linewidth=0.5, zorder=0)
    ax.axvline(0, color="#888", linewidth=0.5, zorder=0)
    ax.grid(True, alpha=0.2)

    basins = analysis.basin_map.get("basins", [])
    for i, root in enumerate(analysis.roots):
        frac = 0.0
        if i < len(basins):
            frac = basins[i].get("fraction", 0.0)
        size = max(80, 800 * frac)
        color = BASIN_COLORS[i % len(BASIN_COLORS)]
        ax.scatter(root["x"], root["y"], s=size, c=color,
                   edgecolors="black", linewidths=1.5, zorder=5)
        label = f"r{i} ({frac*100:.0f}%)"
        ax.annotate(label, (root["x"], root["y"]),
                    textcoords="offset points", xytext=(8, 8),
                    fontsize=9, fontweight="bold", color=color)

    # Draw conjugate pair connections
    rg = analysis.root_geometry
    if rg.get("has_real_axis_symmetry"):
        used = set()
        for i, ri in enumerate(analysis.roots):
            for j, rj in enumerate(analysis.roots):
                if j <= i or i in used or j in used:
                    continue
                if abs(ri["x"] - rj["x"]) < 1e-3 and abs(ri["y"] + rj["y"]) < 1e-3:
                    ax.plot([ri["x"], rj["x"]], [ri["y"], rj["y"]],
                            "--", color="#aaa", linewidth=1, zorder=1)
                    used.update([i, j])

    fig.tight_layout()
    fig.savefig(str(out_path), dpi=150, bbox_inches="tight")
    plt.close(fig)


def chart_basin_pie(analysis: FindingAnalysis, out_path: Path) -> None:
    """Pie chart of basin area shares."""
    import matplotlib.pyplot as plt

    basins = analysis.basin_map.get("basins", [])
    if not basins:
        return

    labels = []
    sizes = []
    colors = []
    for b in basins:
        idx = b["root_index"]
        frac = b["fraction"]
        if frac < 0.001:
            continue
        r = b["root"]
        labels.append(f"Basin {idx}\n({r['x']:.2f}{r['y']:+.2f}i)")
        sizes.append(frac)
        colors.append(BASIN_COLORS[idx % len(BASIN_COLORS)])

    unconv = analysis.basin_map.get("unconverged_fraction", 0)
    if unconv > 0.001:
        labels.append(f"Unconverged\n({unconv*100:.0f}%)")
        sizes.append(unconv)
        colors.append(UNCONVERGED_COLOR)

    fig, ax = plt.subplots(figsize=(7, 5))
    wedges, texts, autotexts = ax.pie(
        sizes, labels=labels, colors=colors, autopct="%1.1f%%",
        startangle=140, pctdistance=0.75,
        textprops={"fontsize": 9})
    for t in autotexts:
        t.set_fontsize(8)
        t.set_fontweight("bold")
        t.set_color("white")
    ax.set_title("Basin Area Distribution", fontsize=14, fontweight="bold")
    fig.tight_layout()
    fig.savefig(str(out_path), dpi=150, bbox_inches="tight")
    plt.close(fig)


def chart_color_palette(analysis: FindingAnalysis, out_path: Path) -> None:
    """Horizontal color swatches of dominant clusters."""
    import matplotlib.pyplot as plt
    import matplotlib.patches as mpatches

    cc = analysis.color_clusters
    if not cc:
        return
    clusters = cc.get("clusters", [])[:8]
    if not clusters:
        return

    fig, ax = plt.subplots(figsize=(8, 2.5))
    ax.set_xlim(0, len(clusters))
    ax.set_ylim(0, 1)
    ax.set_title("Dominant Color Palette", fontsize=14, fontweight="bold")
    ax.set_yticks([])

    for i, cl in enumerate(clusters):
        rgb = cl["center_rgb"]
        color = (rgb[0]/255, rgb[1]/255, rgb[2]/255)
        rect = mpatches.FancyBboxPatch(
            (i + 0.05, 0.1), 0.9, 0.6,
            boxstyle="round,pad=0.05",
            facecolor=color, edgecolor="black", linewidth=1)
        ax.add_patch(rect)
        pct = cl["fraction"] * 100
        text_color = "white" if sum(rgb) < 384 else "black"
        ax.text(i + 0.5, 0.4, f"{pct:.0f}%",
                ha="center", va="center", fontsize=10,
                fontweight="bold", color=text_color)
        ax.text(i + 0.5, 0.82,
                f"({rgb[0]},{rgb[1]},{rgb[2]})",
                ha="center", va="center", fontsize=7, color="#444")

    ax.set_xticks([])
    fig.tight_layout()
    fig.savefig(str(out_path), dpi=150, bbox_inches="tight")
    plt.close(fig)


def chart_convergence_heatmap(analysis: FindingAnalysis, out_path: Path,
                               grid_res: int = 256) -> None:
    """Heatmap of Newton iteration counts over the FULL root extent (not viewport zoom)."""
    import matplotlib.pyplot as plt
    import numpy as np

    coeffs = analysis.state.get("params", {}).get("poly_coeffs", [])
    if len(coeffs) != 5:
        return
    max_iter = min(analysis.state.get("params", {}).get("max_iter", 200), 300)

    roots_c = [Complex(r["x"], r["y"]) for r in analysis.roots]
    if not roots_c:
        return

    # Compute extent from root positions (show full structure, not zoomed viewport)
    all_re = [r.x for r in roots_c]
    all_im = [r.y for r in roots_c]
    margin = 0.5
    re_mid = (max(all_re) + min(all_re)) / 2
    im_mid = (max(all_im) + min(all_im)) / 2
    span = max(max(all_re) - min(all_re), max(all_im) - min(all_im), 1.0)
    half = span / 2 + margin

    re_lo, re_hi = re_mid - half, re_mid + half
    im_lo, im_hi = im_mid - half, im_mid + half

    # Build iteration count grid
    iter_grid = np.zeros((grid_res, grid_res), dtype=np.float32)
    basin_grid = np.full((grid_res, grid_res), -1, dtype=np.int32)

    for py in range(grid_res):
        im = im_hi - (im_hi - im_lo) * py / (grid_res - 1)
        for px in range(grid_res):
            re = re_lo + (re_hi - re_lo) * px / (grid_res - 1)
            z, it, conv = newton_iterate(coeffs, Complex(re, im),
                                         max_iter=max_iter)
            iter_grid[py, px] = it
            if conv:
                best_i = -1
                best_d = 1e10
                for ri, r in enumerate(roots_c):
                    d = abs(z - r)
                    if d < best_d:
                        best_d = d
                        best_i = ri
                if best_d < 0.1:
                    basin_grid[py, px] = best_i

    extent = [re_lo, re_hi, im_lo, im_hi]

    fig, axes = plt.subplots(1, 2, figsize=(13, 5.5))

    # Left: iteration count heatmap
    ax = axes[0]
    im_plot = ax.imshow(iter_grid, cmap="inferno", origin="upper", extent=extent)
    ax.set_title("Convergence Speed", fontsize=12, fontweight="bold")
    ax.set_xlabel("Re(z)")
    ax.set_ylabel("Im(z)")
    ax.set_aspect("equal")
    plt.colorbar(im_plot, ax=ax, label="Iterations", shrink=0.8)

    # Right: basin assignment as colored regions
    ax2 = axes[1]
    n_roots = len(analysis.roots)
    rgb_img = np.zeros((grid_res, grid_res, 3), dtype=np.float32)
    unc_rgb = _rgb_tuple(UNCONVERGED_COLOR)
    for py in range(grid_res):
        for px in range(grid_res):
            bi = basin_grid[py, px]
            if bi >= 0:
                rgb_img[py, px] = _rgb_tuple(BASIN_COLORS[bi % len(BASIN_COLORS)])
            else:
                rgb_img[py, px] = unc_rgb

    ax2.imshow(rgb_img, origin="upper", extent=extent)
    ax2.set_title("Basin Regions", fontsize=12, fontweight="bold")
    ax2.set_xlabel("Re(z)")
    ax2.set_ylabel("Im(z)")
    ax2.set_aspect("equal")

    # Mark roots on both panels
    for i, root in enumerate(analysis.roots):
        color = BASIN_COLORS[i % len(BASIN_COLORS)]
        for a in [ax, ax2]:
            a.plot(root["x"], root["y"], "o", markersize=8,
                   markeredgecolor="white", markeredgewidth=2,
                   markerfacecolor=color, zorder=10)

    # Draw viewport rectangle on both panels
    view = analysis.state.get("view", {})
    vx = view.get("center_x", 0.0)
    vy = view.get("center_y", 0.0)
    vz = view.get("zoom", 1.0)
    render = analysis.state.get("render", {})
    rw = render.get("width", 1024)
    rh = render.get("height", 768)
    vhalf_x = 2.0 / vz
    vhalf_y = 2.0 * rh / (vz * rw)
    import matplotlib.patches as mpatches
    for a in [ax, ax2]:
        rect = mpatches.Rectangle(
            (vx - vhalf_x, vy - vhalf_y), 2 * vhalf_x, 2 * vhalf_y,
            linewidth=1.5, edgecolor="white", facecolor="none",
            linestyle="--", zorder=8)
        a.add_patch(rect)

    fig.suptitle("Newton Fractal Structure (full extent)", fontsize=14, fontweight="bold", y=1.02)
    fig.tight_layout()
    fig.savefig(str(out_path), dpi=150, bbox_inches="tight")
    plt.close(fig)


def chart_symmetry_bars(analysis: FindingAnalysis, out_path: Path) -> None:
    """Bar chart of symmetry scores."""
    import matplotlib.pyplot as plt

    sym = analysis.symmetry
    if not sym:
        return

    labels = ["Horizontal\nReflection", "Vertical\nReflection", "Bilateral\n(combined)"]
    values = [
        sym.get("horizontal_reflection", 0),
        sym.get("vertical_reflection", 0),
        sym.get("bilateral", 0),
    ]
    colors = ["#4363d8", "#3cb44b", "#f58231"]

    fig, ax = plt.subplots(figsize=(5, 3.5))
    bars = ax.bar(labels, values, color=colors, edgecolor="black", linewidth=0.8)
    ax.set_ylim(0, 1.05)
    ax.set_ylabel("Score (0-1)")
    ax.set_title("Symmetry Analysis", fontsize=13, fontweight="bold")
    ax.axhline(0.5, color="#ccc", linewidth=0.8, linestyle="--")
    ax.axhline(0.9, color="#ccc", linewidth=0.8, linestyle="--")
    for bar, val in zip(bars, values):
        ax.text(bar.get_x() + bar.get_width() / 2, bar.get_height() + 0.02,
                f"{val:.3f}", ha="center", va="bottom", fontsize=10, fontweight="bold")
    fig.tight_layout()
    fig.savefig(str(out_path), dpi=150, bbox_inches="tight")
    plt.close(fig)


def chart_frame_annotated(analysis: FindingAnalysis, out_path: Path) -> None:
    """Show the captured frame at full size with an info overlay."""
    import matplotlib.pyplot as plt
    from PIL import Image

    frame_path = Path(analysis.finding_dir) / "frame.png"
    if not frame_path.exists():
        return

    img = Image.open(frame_path)
    w, h = img.size

    fig, ax = plt.subplots(figsize=(10, 7.5))
    ax.imshow(img)
    ax.set_xticks([])
    ax.set_yticks([])

    # Build info text
    view = analysis.state.get("view", {})
    params = analysis.state.get("params", {})
    info_lines = [
        f"Finding: {analysis.finding_id}",
        f"Type: {analysis.state.get('fractal_type', '?')}",
        f"Center: ({view.get('center_x', 0):.4f}, {view.get('center_y', 0):.4f})",
        f"Zoom: {view.get('zoom', 1):.1f}",
        f"Coloring: {params.get('coloring_mode', '?')}",
    ]
    if analysis.roots:
        info_lines.append(f"Roots: {len(analysis.roots)}")
    bm = analysis.basin_map
    if bm:
        unconv = bm.get("unconverged_fraction", 0)
        info_lines.append(f"Unconverged: {unconv*100:.0f}%")

    info_text = "\n".join(info_lines)
    ax.text(0.02, 0.98, info_text, transform=ax.transAxes,
            fontsize=9, verticalalignment="top",
            bbox=dict(boxstyle="round,pad=0.4", facecolor="black", alpha=0.7),
            color="white", fontfamily="monospace")

    fig.tight_layout(pad=0.5)
    fig.savefig(str(out_path), dpi=150, bbox_inches="tight")
    plt.close(fig)


# ---------------------------------------------------------------------------
# CSV export
# ---------------------------------------------------------------------------

def write_csv_roots(analysis: FindingAnalysis, out_path: Path) -> None:
    basins = {b["root_index"]: b for b in analysis.basin_map.get("basins", [])}
    with open(out_path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["root_index", "re", "im", "basin_fraction", "mean_iterations",
                     "conjugate_of"])
        roots = analysis.roots
        # Find conjugate pairs
        conj_map = {}
        for i, ri in enumerate(roots):
            for j, rj in enumerate(roots):
                if j <= i:
                    continue
                if abs(ri["x"] - rj["x"]) < 1e-3 and abs(ri["y"] + rj["y"]) < 1e-3:
                    conj_map[i] = j
                    conj_map[j] = i
        for i, r in enumerate(roots):
            b = basins.get(i, {})
            w.writerow([
                i, f"{r['x']:.6f}", f"{r['y']:.6f}",
                f"{b.get('fraction', 0):.4f}",
                f"{b.get('mean_iterations', 0):.1f}",
                conj_map.get(i, ""),
            ])


def write_csv_basins(analysis: FindingAnalysis, out_path: Path) -> None:
    bm = analysis.basin_map
    with open(out_path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["basin", "root_re", "root_im", "pixel_count", "fraction",
                     "mean_iterations"])
        for b in bm.get("basins", []):
            w.writerow([
                b["root_index"], f"{b['root']['x']:.6f}", f"{b['root']['y']:.6f}",
                b["pixel_count"], f"{b['fraction']:.4f}",
                f"{b['mean_iterations']:.1f}",
            ])
        w.writerow([
            "unconverged", "", "", bm.get("unconverged_count", 0),
            f"{bm.get('unconverged_fraction', 0):.4f}", "",
        ])


def write_csv_frame_metrics(analysis: FindingAnalysis, out_path: Path) -> None:
    fm = analysis.frame_metrics
    if not fm:
        return
    ch = fm.get("channel_means", {})
    sym = analysis.symmetry or {}
    with open(out_path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["metric", "value"])
        w.writerow(["mean_luma", fm.get("mean_luma", "")])
        w.writerow(["std_luma", fm.get("std_luma", "")])
        w.writerow(["edge_energy", fm.get("edge_energy", "")])
        w.writerow(["r_mean", ch.get("r", "")])
        w.writerow(["g_mean", ch.get("g", "")])
        w.writerow(["b_mean", ch.get("b", "")])
        w.writerow(["unique_colors_4bit", fm.get("unique_colors_4bit", "")])
        w.writerow(["symmetry_horizontal", sym.get("horizontal_reflection", "")])
        w.writerow(["symmetry_vertical", sym.get("vertical_reflection", "")])
        w.writerow(["symmetry_bilateral", sym.get("bilateral", "")])


# ---------------------------------------------------------------------------
# Summary markdown
# ---------------------------------------------------------------------------

def write_summary_md(analysis: FindingAnalysis, out_dir: Path, out_path: Path) -> None:
    """Generate a self-contained markdown summary with relative image links."""
    a = analysis
    lines = []
    lines.append(f"# Finding Analysis: {a.finding_id}")
    lines.append("")
    lines.append(f"**Fractal type:** {a.state.get('fractal_type', 'unknown')}")
    lines.append(f"**Source:** `{a.finding_dir}`")
    lines.append("")

    # State overview
    view = a.state.get("view", {})
    params = a.state.get("params", {})
    lines.append("## State")
    lines.append(f"- Center: ({view.get('center_x', 0):.6f}, {view.get('center_y', 0):.6f})")
    lines.append(f"- Zoom: {view.get('zoom', 1):.2f} (log2 = {view.get('log2_zoom', 0):.3f})")

    coeffs = params.get("poly_coeffs", [])
    if coeffs:
        terms = []
        for i, c in enumerate(coeffs):
            if abs(c) < 1e-10:
                continue
            if i == 0:
                terms.append(f"{c:.4f}")
            elif i == 1:
                terms.append(f"{c:+.4f}z")
            else:
                terms.append(f"{c:+.4f}z^{i}")
        lines.append(f"- Polynomial: p(z) = {''.join(terms)}")
    lines.append(f"- Coloring: {params.get('coloring_mode', 'unknown')}")
    lines.append(f"- Max iterations: {params.get('max_iter', 0)}")
    lines.append("")

    # Annotated frame
    lines.append("## Fractal View")
    lines.append("")
    lines.append("![Annotated frame](frame_annotated.png)")
    lines.append("")

    # Roots table
    lines.append("## Roots")
    lines.append("")
    lines.append("![Root constellation](root_constellation.png)")
    lines.append("")
    lines.append("| Root | Re | Im | Basin % | Mean Iters |")
    lines.append("|------|----|----|---------|------------|")
    basins = {b["root_index"]: b for b in a.basin_map.get("basins", [])}
    for i, r in enumerate(a.roots):
        b = basins.get(i, {})
        frac = b.get("fraction", 0) * 100
        iters = b.get("mean_iterations", 0)
        lines.append(f"| r{i} | {r['x']:+.4f} | {r['y']:+.4f} | {frac:.1f}% | {iters:.0f} |")
    unconv = a.basin_map.get("unconverged_fraction", 0) * 100
    if unconv > 0:
        lines.append(f"| -- | -- | -- | {unconv:.1f}% unconverged | -- |")
    lines.append("")

    rg = a.root_geometry
    if rg:
        lines.append(f"**Conjugate pairs:** {rg.get('conjugate_pairs', 0)} | "
                      f"**Mean radius:** {rg.get('mean_radius', 0):.3f} | "
                      f"**Spread:** {rg.get('radius_spread', 0):.3f}")
        lines.append("")

    # Basins
    lines.append("## Basin Structure")
    lines.append("")
    lines.append("![Basin pie](basin_pie.png)")
    lines.append("")

    # Convergence
    lines.append("## Convergence Analysis")
    lines.append("")
    lines.append("![Convergence heatmap](convergence_heatmap.png)")
    lines.append("")

    # Symmetry
    lines.append("## Symmetry")
    lines.append("")
    lines.append("![Symmetry bars](symmetry_bars.png)")
    lines.append("")
    sym = a.symmetry or {}
    lines.append(f"- Horizontal: {sym.get('horizontal_reflection', 0):.3f}")
    lines.append(f"- Vertical: {sym.get('vertical_reflection', 0):.3f}")
    lines.append(f"- Bilateral: {sym.get('bilateral', 0):.3f}")
    lines.append("")

    # Color palette
    lines.append("## Color Palette")
    lines.append("")
    lines.append("![Color palette](color_palette.png)")
    lines.append("")

    # Frame metrics
    fm = a.frame_metrics
    if fm:
        lines.append("## Frame Metrics")
        ch = fm.get("channel_means", {})
        lines.append(f"- Mean luma: {fm.get('mean_luma', 0):.1f} (std: {fm.get('std_luma', 0):.1f})")
        lines.append(f"- Edge energy: {fm.get('edge_energy', 0):.3f}")
        lines.append(f"- Channels: R={ch.get('r',0):.0f} G={ch.get('g',0):.0f} B={ch.get('b',0):.0f}")
        lines.append(f"- Unique colors (4-bit): {fm.get('unique_colors_4bit', 0)}")
    lines.append("")

    # Highlights
    lines.append("## Key Highlights")
    lines.append("")
    highlights = _generate_highlights(a)
    for h_line in highlights:
        lines.append(f"- {h_line}")
    lines.append("")

    # CSV index
    lines.append("## Data Files")
    lines.append("")
    lines.append("| File | Description |")
    lines.append("|------|-------------|")
    lines.append("| [roots.csv](roots.csv) | Root positions and basin fractions |")
    lines.append("| [basins.csv](basins.csv) | Basin pixel counts and iteration stats |")
    lines.append("| [frame_metrics.csv](frame_metrics.csv) | Frame quality metrics |")
    lines.append("| [analysis.json](analysis.json) | Full analysis data (machine-readable) |")
    lines.append("")

    out_path.write_text("\n".join(lines), encoding="utf-8")


def _generate_highlights(a: FindingAnalysis) -> list:
    """Auto-generate the top findings as bullet points."""
    highlights = []

    # Root structure
    rg = a.root_geometry
    n = rg.get("n_roots", 0)
    cp = rg.get("conjugate_pairs", 0)
    if cp > 0:
        highlights.append(f"{n} roots arranged in {cp} conjugate pair(s) -- real-axis symmetry")
    else:
        highlights.append(f"{n} roots with no conjugate symmetry")

    # Basin dominance
    basins = a.basin_map.get("basins", [])
    if basins:
        best = max(basins, key=lambda b: b["fraction"])
        highlights.append(
            f"Dominant basin: root {best['root_index']} "
            f"at ({best['root']['x']:.2f}, {best['root']['y']:.2f}) "
            f"with {best['fraction']*100:.0f}% area")
        min_b = min(basins, key=lambda b: b["fraction"])
        if min_b["fraction"] < 0.05:
            highlights.append(
                f"Smallest basin (root {min_b['root_index']}) covers only "
                f"{min_b['fraction']*100:.1f}% -- nearly invisible")

    # Unconverged
    unconv = a.basin_map.get("unconverged_fraction", 0)
    if unconv > 0.30:
        highlights.append(f"Large unconverged region ({unconv*100:.0f}%) -- dark lobes indicate chaotic dynamics")
    elif unconv > 0.10:
        highlights.append(f"Moderate unconverged region ({unconv*100:.0f}%)")
    elif unconv > 0:
        highlights.append(f"Low unconverged fraction ({unconv*100:.1f}%) -- clean basin boundaries")

    # Symmetry
    sym = a.symmetry or {}
    bilateral = sym.get("bilateral", 0)
    if bilateral > 0.85:
        highlights.append(f"High bilateral symmetry ({bilateral:.3f}) -- consistent with conjugate pairs and zero warp")
    elif bilateral > 0.6:
        highlights.append(f"Moderate bilateral symmetry ({bilateral:.3f})")

    # Frame
    fm = a.frame_metrics
    if fm:
        ee = fm.get("edge_energy", 0)
        if ee > 5:
            highlights.append("High edge energy -- rich fractal detail / thin basin boundaries")
        elif ee > 2:
            highlights.append(f"Moderate edge energy ({ee:.2f}) -- visible but not extreme detail")

    return highlights


# ---------------------------------------------------------------------------
# Main orchestrator
# ---------------------------------------------------------------------------

def generate_all(analysis: FindingAnalysis, out_dir: Path,
                 convergence_grid: int = 200) -> dict:
    """Generate all charts, CSVs, and summary. Returns manifest of output files."""
    out_dir.mkdir(parents=True, exist_ok=True)
    manifest = {}

    import json
    from dataclasses import asdict

    # JSON
    json_path = out_dir / "analysis.json"
    json_path.write_text(json.dumps(asdict(analysis), indent=2))
    manifest["analysis_json"] = str(json_path)

    # CSVs
    csv_roots = out_dir / "roots.csv"
    write_csv_roots(analysis, csv_roots)
    manifest["roots_csv"] = str(csv_roots)

    csv_basins = out_dir / "basins.csv"
    write_csv_basins(analysis, csv_basins)
    manifest["basins_csv"] = str(csv_basins)

    csv_fm = out_dir / "frame_metrics.csv"
    write_csv_frame_metrics(analysis, csv_fm)
    manifest["frame_metrics_csv"] = str(csv_fm)

    # Charts
    print("  Generating root constellation...", flush=True)
    rc_path = out_dir / "root_constellation.png"
    chart_root_constellation(analysis, rc_path)
    manifest["root_constellation"] = str(rc_path)

    print("  Generating basin pie chart...", flush=True)
    bp_path = out_dir / "basin_pie.png"
    chart_basin_pie(analysis, bp_path)
    manifest["basin_pie"] = str(bp_path)

    print("  Generating color palette...", flush=True)
    cp_path = out_dir / "color_palette.png"
    chart_color_palette(analysis, cp_path)
    manifest["color_palette"] = str(cp_path)

    print("  Generating symmetry bars...", flush=True)
    sb_path = out_dir / "symmetry_bars.png"
    chart_symmetry_bars(analysis, sb_path)
    manifest["symmetry_bars"] = str(sb_path)

    print("  Generating convergence heatmap (this may take a moment)...", flush=True)
    ch_path = out_dir / "convergence_heatmap.png"
    chart_convergence_heatmap(analysis, ch_path, grid_res=convergence_grid)
    manifest["convergence_heatmap"] = str(ch_path)

    print("  Generating annotated frame...", flush=True)
    af_path = out_dir / "frame_annotated.png"
    chart_frame_annotated(analysis, af_path)
    manifest["frame_annotated"] = str(af_path)

    # Summary markdown
    print("  Writing summary...", flush=True)
    summary_path = out_dir / "summary.md"
    write_summary_md(analysis, out_dir, summary_path)
    manifest["summary_md"] = str(summary_path)

    # Text report
    report_path = out_dir / "report.txt"
    report_path.write_text(format_report(analysis))
    manifest["report_txt"] = str(report_path)

    return manifest
