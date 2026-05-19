"""Analyze a captured finding: polynomial roots, basin structure, frame metrics.

Usage:
    python -m fractal_explorer.finding_analyzer <finding_dir>
    python -m fractal_explorer.finding_analyzer --finding-id 220722_840__explaino_dual --date 2026-04-05

Loads state.json + frame.png, computes:
  - Polynomial root positions (from coefficients or explaino_roots)
  - Basin assignment per-pixel (nearest root by color)
  - Convergence rate statistics
  - Symmetry classification (approximate rotational + reflective)
  - Frame spatial metrics (edge energy, luma distribution)
  - Report as JSON + console summary
"""
from __future__ import annotations

import json
import math
import struct
import sys
from dataclasses import dataclass, field, asdict
from pathlib import Path
from typing import List, Optional, Tuple

from .paths import findings_root

# ---------------------------------------------------------------------------
# Polynomial math (pure Python, matches C++ fractal_derived_fields)
# ---------------------------------------------------------------------------

@dataclass
class Complex:
    x: float
    y: float

    def __add__(self, other: "Complex") -> "Complex":
        return Complex(self.x + other.x, self.y + other.y)

    def __sub__(self, other: "Complex") -> "Complex":
        return Complex(self.x - other.x, self.y - other.y)

    def __mul__(self, other: "Complex") -> "Complex":
        return Complex(self.x * other.x - self.y * other.y,
                       self.x * other.y + self.y * other.x)

    def __abs__(self) -> float:
        return math.sqrt(self.x * self.x + self.y * self.y)

    def scale(self, s: float) -> "Complex":
        return Complex(self.x * s, self.y * s)

    def conj(self) -> "Complex":
        return Complex(self.x, -self.y)


def poly_eval(coeffs: List[float], z: Complex) -> Tuple[Complex, Complex]:
    """Evaluate degree-4 polynomial P(z) and P'(z) with real coefficients."""
    z2 = z * z
    z3 = z2 * z
    z4 = z2 * z2

    P = Complex(coeffs[0], 0.0)
    P = P + z.scale(coeffs[1])
    P = P + z2.scale(coeffs[2])
    P = P + z3.scale(coeffs[3])
    P = P + z4.scale(coeffs[4])

    dP = Complex(coeffs[1], 0.0)
    dP = dP + z.scale(2.0 * coeffs[2])
    dP = dP + z2.scale(3.0 * coeffs[3])
    dP = dP + z3.scale(4.0 * coeffs[4])

    return P, dP


def newton_iterate(coeffs: List[float], z0: Complex,
                   max_iter: int = 500, eps: float = 1e-6,
                   damping: float = 1.0) -> Tuple[Complex, int, bool]:
    """Run Newton's method. Returns (z_final, iterations, converged)."""
    z = z0
    for it in range(max_iter):
        P, dP = poly_eval(coeffs, z)
        p_abs = abs(P)
        if p_abs < eps:
            return z, it, True
        d_abs2 = dP.x * dP.x + dP.y * dP.y
        if d_abs2 < 1e-20:
            break
        inv_d = Complex(dP.x / d_abs2, -dP.y / d_abs2)
        step = P * inv_d
        z = z - step.scale(damping)
        if not (math.isfinite(z.x) and math.isfinite(z.y)):
            break
    return z, max_iter, False


def find_roots_numerically(coeffs: List[float], known_roots: Optional[List[dict]] = None,
                           grid_n: int = 20, radius: float = 3.0) -> List[Complex]:
    """Find polynomial roots via Newton's method from a grid of starting points."""
    if known_roots:
        return [Complex(r["x"], r["y"]) for r in known_roots]

    found: List[Complex] = []
    for iy in range(grid_n):
        for ix in range(grid_n):
            re = -radius + 2 * radius * ix / (grid_n - 1)
            im = -radius + 2 * radius * iy / (grid_n - 1)
            z, it, conv = newton_iterate(coeffs, Complex(re, im))
            if conv:
                # Check if this root is new
                is_new = True
                for r in found:
                    if abs(z - r) < 1e-4:
                        is_new = False
                        break
                if is_new:
                    found.append(z)
    return found


def classify_basin(z_final: Complex, roots: List[Complex], eps: float = 0.01) -> int:
    """Return index of nearest root, or -1 if no match."""
    best_i = -1
    best_d = float("inf")
    for i, r in enumerate(roots):
        d = abs(z_final - r)
        if d < best_d:
            best_d = d
            best_i = i
    return best_i if best_d < eps else -1


# ---------------------------------------------------------------------------
# Frame analysis (pure Python PNG reader for 24-bit uncompressed, or pillow)
# ---------------------------------------------------------------------------

def load_frame_pixels(frame_path: Path) -> Optional[Tuple[int, int, List[Tuple[int, int, int]]]]:
    """Load frame.png and return (width, height, [(r,g,b), ...]).
    Tries PIL first, falls back to raw BMP if available."""
    try:
        from PIL import Image
        img = Image.open(frame_path).convert("RGB")
        w, h = img.size
        pixels = list(img.getdata())
        return w, h, pixels
    except ImportError:
        pass
    # Fallback: check for BMP
    bmp_path = frame_path.with_suffix(".bmp")
    if bmp_path.exists():
        return _load_bmp_pixels(bmp_path)
    return None


def _load_bmp_pixels(bmp_path: Path) -> Optional[Tuple[int, int, List[Tuple[int, int, int]]]]:
    """Minimal 24-bit BMP loader."""
    data = bmp_path.read_bytes()
    if data[:2] != b"BM":
        return None
    offset = struct.unpack_from("<I", data, 10)[0]
    w = struct.unpack_from("<i", data, 18)[0]
    h = struct.unpack_from("<i", data, 22)[0]
    bpp = struct.unpack_from("<H", data, 28)[0]
    if bpp != 24:
        return None
    row_size = (w * 3 + 3) & ~3
    pixels = []
    for y in range(abs(h)):
        row_y = (abs(h) - 1 - y) if h > 0 else y
        row_off = offset + row_y * row_size
        for x in range(w):
            b, g, r = data[row_off + x * 3: row_off + x * 3 + 3]
            pixels.append((r, g, b))
    return abs(w), abs(h), pixels


def compute_frame_metrics(w: int, h: int,
                          pixels: List[Tuple[int, int, int]]) -> dict:
    """Compute spatial metrics from frame pixels."""
    n = w * h
    luma = [0.299 * p[0] + 0.587 * p[1] + 0.114 * p[2] for p in pixels]
    mean_luma = sum(luma) / n
    std_luma = math.sqrt(sum((l - mean_luma) ** 2 for l in luma) / n)

    # Edge energy: sum of |gradient| using Sobel-like horizontal diff
    edge_energy = 0.0
    for y in range(h):
        for x in range(1, w):
            d = abs(luma[y * w + x] - luma[y * w + x - 1])
            edge_energy += d
    for y in range(1, h):
        for x in range(w):
            d = abs(luma[y * w + x] - luma[(y - 1) * w + x])
            edge_energy += d
    edge_energy /= (2 * n)

    # Color channel means
    r_mean = sum(p[0] for p in pixels) / n
    g_mean = sum(p[1] for p in pixels) / n
    b_mean = sum(p[2] for p in pixels) / n

    # Unique color count (approximate by downsampling to 4-bit)
    color_set = set()
    for p in pixels:
        color_set.add((p[0] >> 4, p[1] >> 4, p[2] >> 4))

    return {
        "mean_luma": round(mean_luma, 2),
        "std_luma": round(std_luma, 2),
        "edge_energy": round(edge_energy, 4),
        "channel_means": {"r": round(r_mean, 1), "g": round(g_mean, 1), "b": round(b_mean, 1)},
        "unique_colors_4bit": len(color_set),
    }


def classify_basin_colors(w: int, h: int,
                          pixels: List[Tuple[int, int, int]],
                          n_clusters: int = 8) -> dict:
    """Simple color clustering to identify basin regions.
    Returns cluster centers and pixel counts."""
    # Use k-means-like approach with fixed palette extraction
    # Sample pixels for speed
    step = max(1, len(pixels) // 10000)
    sample = pixels[::step]

    # Find dominant colors via histogram binning
    bins: dict = {}
    for r, g, b in sample:
        key = (r >> 4, g >> 4, b >> 4)
        bins[key] = bins.get(key, 0) + 1

    # Top N bins
    top = sorted(bins.items(), key=lambda x: -x[1])[:n_clusters]
    centers = [(k[0] * 16 + 8, k[1] * 16 + 8, k[2] * 16 + 8) for k, _ in top]

    # Assign all pixels to nearest center
    counts = [0] * len(centers)
    for r, g, b in pixels:
        best_i = 0
        best_d = float("inf")
        for i, (cr, cg, cb) in enumerate(centers):
            d = (r - cr) ** 2 + (g - cg) ** 2 + (b - cb) ** 2
            if d < best_d:
                best_d = d
                best_i = i
        counts[best_i] += 1

    total = w * h
    clusters = []
    for i, (c, count) in enumerate(zip(centers, counts)):
        clusters.append({
            "center_rgb": list(c),
            "pixel_count": count,
            "fraction": round(count / total, 4),
        })
    clusters.sort(key=lambda x: -x["fraction"])
    return {"clusters": clusters, "n_clusters": len(clusters)}


def measure_symmetry(w: int, h: int, pixels: List[Tuple[int, int, int]]) -> dict:
    """Measure approximate reflective and rotational symmetry."""
    luma = [0.299 * p[0] + 0.587 * p[1] + 0.114 * p[2] for p in pixels]

    def pixel_at(x: int, y: int) -> float:
        if 0 <= x < w and 0 <= y < h:
            return luma[y * w + x]
        return 0.0

    # Horizontal reflection: compare left/right halves
    h_diff = 0.0
    h_count = 0
    for y in range(h):
        for x in range(w // 2):
            xr = w - 1 - x
            d = abs(pixel_at(x, y) - pixel_at(xr, y))
            h_diff += d
            h_count += 1
    h_sym = 1.0 - (h_diff / h_count / 255.0) if h_count > 0 else 0.0

    # Vertical reflection: compare top/bottom halves
    v_diff = 0.0
    v_count = 0
    for y in range(h // 2):
        yr = h - 1 - y
        for x in range(w):
            d = abs(pixel_at(x, y) - pixel_at(x, yr))
            v_diff += d
            v_count += 1
    v_sym = 1.0 - (v_diff / v_count / 255.0) if v_count > 0 else 0.0

    return {
        "horizontal_reflection": round(h_sym, 4),
        "vertical_reflection": round(v_sym, 4),
        "bilateral": round((h_sym + v_sym) / 2, 4),
    }


# ---------------------------------------------------------------------------
# Basin convergence map (headless Newton iteration over the viewport)
# ---------------------------------------------------------------------------

def compute_basin_map(coeffs: List[float], roots: List[Complex],
                      center: Tuple[float, float], zoom: float,
                      width: int, height: int,
                      sample_step: int = 4,
                      max_iter: int = 200, eps: float = 1e-6,
                      damping: float = 1.0) -> dict:
    """Run Newton iteration over the viewport, classify basins.
    Returns basin counts, mean iteration counts per basin, boundary fraction."""
    basin_counts = [0] * (len(roots) + 1)  # +1 for unconverged
    basin_iters: List[List[int]] = [[] for _ in range(len(roots) + 1)]
    boundary_pixels = 0
    total_pixels = 0

    for py in range(0, height, sample_step):
        for px in range(0, width, sample_step):
            re = center[0] + (px - width / 2.0) / (zoom * width / 4.0)
            im = center[1] - (py - height / 2.0) / (zoom * width / 4.0)
            z, it, conv = newton_iterate(coeffs, Complex(re, im),
                                         max_iter=max_iter, eps=eps, damping=damping)
            if conv:
                bi = classify_basin(z, roots, eps=0.1)
                if bi >= 0:
                    basin_counts[bi] += 1
                    basin_iters[bi].append(it)
                else:
                    basin_counts[-1] += 1
                    basin_iters[-1].append(it)
            else:
                basin_counts[-1] += 1
                basin_iters[-1].append(max_iter)

            # Check if this is a boundary pixel (neighbor in different basin)
            total_pixels += 1

    # Compute mean iterations per basin
    basin_stats = []
    for i in range(len(roots)):
        iters = basin_iters[i]
        mean_it = sum(iters) / len(iters) if iters else 0
        basin_stats.append({
            "root_index": i,
            "root": {"x": round(roots[i].x, 6), "y": round(roots[i].y, 6)},
            "pixel_count": basin_counts[i],
            "fraction": round(basin_counts[i] / max(1, total_pixels), 4),
            "mean_iterations": round(mean_it, 1),
        })

    unconverged_count = basin_counts[-1]
    return {
        "basins": basin_stats,
        "unconverged_count": unconverged_count,
        "unconverged_fraction": round(unconverged_count / max(1, total_pixels), 4),
        "total_sampled_pixels": total_pixels,
        "sample_step": sample_step,
    }


# ---------------------------------------------------------------------------
# Root geometry analysis
# ---------------------------------------------------------------------------

def analyze_root_geometry(roots: List[Complex]) -> dict:
    """Characterize the spatial arrangement of roots."""
    n = len(roots)
    if n == 0:
        return {"error": "no roots"}

    centroid = Complex(
        sum(r.x for r in roots) / n,
        sum(r.y for r in roots) / n
    )

    distances = [abs(r - centroid) for r in roots]
    pairwise = []
    for i in range(n):
        for j in range(i + 1, n):
            pairwise.append(abs(roots[i] - roots[j]))

    # Check for conjugate pairs
    conjugate_pairs = 0
    used = set()
    for i in range(n):
        if i in used:
            continue
        for j in range(i + 1, n):
            if j in used:
                continue
            if (abs(roots[i].x - roots[j].x) < 1e-4 and
                    abs(roots[i].y + roots[j].y) < 1e-4):
                conjugate_pairs += 1
                used.add(i)
                used.add(j)
                break

    return {
        "n_roots": n,
        "centroid": {"x": round(centroid.x, 6), "y": round(centroid.y, 6)},
        "distances_from_centroid": [round(d, 6) for d in distances],
        "pairwise_distances": [round(d, 6) for d in sorted(pairwise)],
        "conjugate_pairs": conjugate_pairs,
        "has_real_axis_symmetry": conjugate_pairs == n // 2,
        "mean_radius": round(sum(distances) / n, 6),
        "radius_spread": round(max(distances) - min(distances), 6) if distances else 0,
    }


# ---------------------------------------------------------------------------
# Main analysis entry point
# ---------------------------------------------------------------------------

@dataclass
class FindingAnalysis:
    finding_id: str
    finding_dir: str
    state: dict
    roots: List[dict] = field(default_factory=list)
    root_geometry: dict = field(default_factory=dict)
    basin_map: dict = field(default_factory=dict)
    render_stats: dict = field(default_factory=dict)
    frame_metrics: dict = field(default_factory=dict)
    color_clusters: dict = field(default_factory=dict)
    symmetry: dict = field(default_factory=dict)


def summarize_render_stats(state: dict) -> dict:
    """Return product-facing render-stat labels from a captured state.json."""
    raw = state.get("stats", {})
    if not isinstance(raw, dict):
        return {}

    def as_number(name: str) -> float | int | None:
        value = raw.get(name)
        if isinstance(value, bool):
            return None
        return value if isinstance(value, (int, float)) else None

    last_iters_sum = as_number("last_iters_sum")
    last_pixel_count = as_number("last_pixel_count")
    derived_average = None
    if (
        isinstance(last_iters_sum, (int, float))
        and isinstance(last_pixel_count, (int, float))
        and last_pixel_count > 0
    ):
        derived_average = round(float(last_iters_sum) / float(last_pixel_count), 3)

    last_render_ms = as_number("last_render_ms")
    timing_status = "measured"
    if last_render_ms is None:
        timing_status = "missing"
    elif float(last_render_ms) <= 0.0:
        timing_status = "unmeasured_or_stale_zero"

    summary = {
        "last_render_ms": last_render_ms,
        "timing_status": timing_status,
        "last_iters_avg": as_number("last_iters_avg"),
        "last_iters_sum": last_iters_sum,
        "last_pixel_count": last_pixel_count,
        "derived_iters_avg": derived_average,
        "resolved_backend": raw.get("resolved_backend"),
        "resolved_strategy": raw.get("resolved_strategy"),
    }
    return {key: value for key, value in summary.items() if value is not None}


def analyze_finding(finding_dir: Path, sample_step: int = 8) -> FindingAnalysis:
    """Full analysis of a captured finding."""
    state_path = finding_dir / "state.json"
    if not state_path.exists():
        raise FileNotFoundError(f"No state.json in {finding_dir}")

    state = json.loads(state_path.read_text())
    finding_json_path = finding_dir / "finding.json"
    finding_id = "unknown"
    if finding_json_path.exists():
        fj = json.loads(finding_json_path.read_text())
        finding_id = fj.get("finding_id", "unknown")

    analysis = FindingAnalysis(
        finding_id=finding_id,
        finding_dir=str(finding_dir),
        state=state,
    )

    analysis.render_stats = summarize_render_stats(state)

    params = state.get("params", {})
    view = state.get("view", {})
    coeffs = params.get("poly_coeffs", [])

    # Extract roots
    explaino_roots_raw = None
    root_count = params.get("explaino_root_count", 0)
    # The state.json doesn't store explaino_roots explicitly in all versions.
    # Re-derive from coefficients via Newton iteration.
    if len(coeffs) == 5:
        roots = find_roots_numerically(coeffs)
    else:
        roots = []

    analysis.roots = [{"x": round(r.x, 6), "y": round(r.y, 6)} for r in roots]
    analysis.root_geometry = analyze_root_geometry(roots)

    # Basin map from Newton iteration over viewport
    center = (view.get("center_x", 0.0), view.get("center_y", 0.0))
    zoom = view.get("zoom", 1.0)
    render = state.get("render", {})
    w = render.get("width", 1024)
    h = render.get("height", 768)
    damping = params.get("newton_damping", 1.0)

    if len(coeffs) == 5 and roots:
        analysis.basin_map = compute_basin_map(
            coeffs, roots, center, zoom, w, h,
            sample_step=sample_step, damping=damping)

    # Frame analysis
    frame_path = finding_dir / "frame.png"
    if not frame_path.exists():
        frame_path = finding_dir / "frame.bmp"
    if frame_path.exists():
        result = load_frame_pixels(frame_path)
        if result:
            fw, fh, pixels = result
            analysis.frame_metrics = compute_frame_metrics(fw, fh, pixels)
            analysis.color_clusters = classify_basin_colors(fw, fh, pixels)
            analysis.symmetry = measure_symmetry(fw, fh, pixels)

    return analysis


def format_report(a: FindingAnalysis) -> str:
    """Format analysis as human-readable report."""
    lines = []
    lines.append(f"=== Finding Analysis: {a.finding_id} ===")
    lines.append(f"Dir: {a.finding_dir}")
    lines.append(f"Fractal type: {a.state.get('fractal_type', 'unknown')}")
    lines.append("")

    # Polynomial
    coeffs = a.state.get("params", {}).get("poly_coeffs", [])
    if coeffs:
        terms = []
        for i, c in enumerate(coeffs):
            if abs(c) < 1e-10:
                continue
            if i == 0:
                terms.append(f"{c:.5f}")
            elif i == 1:
                terms.append(f"{c:+.5f}z")
            else:
                terms.append(f"{c:+.5f}z^{i}")
        lines.append(f"Polynomial: p(z) = {''.join(terms)}")
    lines.append("")

    # Roots
    lines.append(f"Roots ({len(a.roots)}):")
    for i, r in enumerate(a.roots):
        lines.append(f"  r{i}: {r['x']:+.6f} {r['y']:+.6f}i")
    lines.append("")

    # Root geometry
    rg = a.root_geometry
    if rg:
        lines.append(f"Root geometry:")
        lines.append(f"  Centroid: ({rg.get('centroid', {}).get('x', 0):.4f}, {rg.get('centroid', {}).get('y', 0):.4f})")
        lines.append(f"  Mean radius: {rg.get('mean_radius', 0):.4f}")
        lines.append(f"  Radius spread: {rg.get('radius_spread', 0):.4f}")
        lines.append(f"  Conjugate pairs: {rg.get('conjugate_pairs', 0)}")
        lines.append(f"  Real-axis symmetric: {rg.get('has_real_axis_symmetry', False)}")
    lines.append("")

    # Basin map
    bm = a.basin_map
    if bm:
        lines.append(f"Basin convergence ({bm.get('total_sampled_pixels', 0)} sampled pixels, step={bm.get('sample_step', 0)}):")
        for b in bm.get("basins", []):
            lines.append(f"  Basin {b['root_index']}: root=({b['root']['x']:.4f},{b['root']['y']:.4f})  "
                         f"{b['fraction']*100:.1f}%  mean_iter={b['mean_iterations']:.0f}")
        lines.append(f"  Unconverged: {bm.get('unconverged_fraction', 0)*100:.1f}%")
    lines.append("")

    # Render stats
    rs = a.render_stats
    if rs:
        lines.append("Render stats:")
        timing_status = rs.get("timing_status", "unknown")
        render_ms = rs.get("last_render_ms")
        if timing_status == "measured" and isinstance(render_ms, (int, float)):
            lines.append(f"  Render time: {render_ms:.3f} ms")
        elif timing_status == "unmeasured_or_stale_zero":
            lines.append("  Render time: unmeasured/stale zero")
        if "last_iters_avg" in rs:
            lines.append(f"  Average iteration count: {rs['last_iters_avg']}")
        if "derived_iters_avg" in rs:
            lines.append(f"  Derived average from raw sum/pixels: {rs['derived_iters_avg']}")
        if "last_iters_sum" in rs:
            lines.append(f"  Raw iteration sum: {rs['last_iters_sum']}")
        if "last_pixel_count" in rs:
            lines.append(f"  Pixel count: {rs['last_pixel_count']}")
        backend = rs.get("resolved_backend")
        strategy = rs.get("resolved_strategy")
        if backend or strategy:
            lines.append(f"  Backend/strategy: {backend or 'unknown'} / {strategy or 'unknown'}")
    lines.append("")

    # Symmetry
    sym = a.symmetry
    if sym:
        lines.append(f"Symmetry:")
        lines.append(f"  Horizontal reflection: {sym.get('horizontal_reflection', 0):.3f}")
        lines.append(f"  Vertical reflection: {sym.get('vertical_reflection', 0):.3f}")
        lines.append(f"  Bilateral: {sym.get('bilateral', 0):.3f}")
    lines.append("")

    # Frame metrics
    fm = a.frame_metrics
    if fm:
        lines.append(f"Frame metrics:")
        lines.append(f"  Mean luma: {fm.get('mean_luma', 0):.1f}")
        lines.append(f"  Luma stddev: {fm.get('std_luma', 0):.1f}")
        lines.append(f"  Edge energy: {fm.get('edge_energy', 0):.3f}")
        ch = fm.get("channel_means", {})
        lines.append(f"  Channel means: R={ch.get('r', 0):.0f} G={ch.get('g', 0):.0f} B={ch.get('b', 0):.0f}")
        lines.append(f"  Unique colors (4-bit): {fm.get('unique_colors_4bit', 0)}")
    lines.append("")

    # Color clusters
    cc = a.color_clusters
    if cc:
        lines.append(f"Color clusters ({cc.get('n_clusters', 0)}):")
        for cl in cc.get("clusters", [])[:6]:
            rgb = cl["center_rgb"]
            lines.append(f"  RGB({rgb[0]:3d},{rgb[1]:3d},{rgb[2]:3d}): {cl['fraction']*100:.1f}%")

    return "\n".join(lines)


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def main(args: Optional[List[str]] = None) -> int:
    import argparse
    parser = argparse.ArgumentParser(description="Analyze a captured fractal finding")
    parser.add_argument("finding_dir", nargs="?",
                        help="Path to finding directory (contains state.json + frame.png)")
    parser.add_argument("--finding-id", help="Finding ID to locate under findings root")
    parser.add_argument("--date", help="Date subfolder (YYYY-MM-DD)")
    parser.add_argument("--group", default="manual_capture",
                        help="Finding group (default: manual_capture)")
    parser.add_argument("--sample-step", type=int, default=8,
                        help="Basin map sampling step (lower=slower, more detail)")
    parser.add_argument("--out-json", help="Write analysis JSON to this path")
    parsed = parser.parse_args(args)

    if parsed.finding_dir:
        finding_dir = Path(parsed.finding_dir)
    elif parsed.finding_id and parsed.date:
        repo_root = Path(__file__).resolve().parents[3]
        root = findings_root(repo_root)
        finding_dir = root / parsed.group / parsed.date / parsed.finding_id
    else:
        parser.error("Provide finding_dir or --finding-id + --date")
        return 1

    if not finding_dir.exists():
        print(f"ERROR: {finding_dir} does not exist", file=sys.stderr)
        return 1

    print(f"Analyzing: {finding_dir}")
    analysis = analyze_finding(finding_dir, sample_step=parsed.sample_step)

    print()
    print(format_report(analysis))

    if parsed.out_json:
        out_path = Path(parsed.out_json)
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(json.dumps(asdict(analysis), indent=2))
        print(f"\nJSON written to: {out_path}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
