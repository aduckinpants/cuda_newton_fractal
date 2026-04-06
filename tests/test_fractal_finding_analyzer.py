"""Tests for finding_analyzer: polynomial root-finding, basin classification, geometry."""
from __future__ import annotations

import json
import math
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools" / "reality_toolkit"))

from fractal_explorer.finding_analyzer import (
    Complex,
    analyze_root_geometry,
    classify_basin,
    compute_basin_map,
    find_roots_numerically,
    format_report,
    newton_iterate,
    poly_eval,
    analyze_finding,
)


def test_poly_eval_z3_minus_1():
    """p(z) = z^3 - 1 at z=1 should give P=0."""
    coeffs = [-1.0, 0.0, 0.0, 1.0, 0.0]
    P, dP = poly_eval(coeffs, Complex(1.0, 0.0))
    assert abs(P) < 1e-10, f"P(1) should be 0, got {abs(P)}"
    assert abs(dP - Complex(3.0, 0.0)) < 1e-10, f"P'(1) should be 3"


def test_newton_converges_z3():
    """Newton on z^3 - 1 starting near (1,0) should converge to (1,0)."""
    coeffs = [-1.0, 0.0, 0.0, 1.0, 0.0]
    z, it, conv = newton_iterate(coeffs, Complex(0.9, 0.1))
    assert conv, f"Should converge, got it={it}"
    assert abs(z - Complex(1.0, 0.0)) < 1e-5, f"Should find root at (1,0), got ({z.x},{z.y})"


def test_find_roots_z4_minus_1():
    """z^4 - 1 has roots at 1, -1, i, -i."""
    coeffs = [-1.0, 0.0, 0.0, 0.0, 1.0]
    roots = find_roots_numerically(coeffs, grid_n=30)
    assert len(roots) == 4, f"Expected 4 roots, got {len(roots)}"


def test_find_roots_explaino_dual():
    """The captured finding's polynomial should yield 4 roots."""
    coeffs = [1.56501, 0.136509, -0.57067, 1.08636, 1.0]
    roots = find_roots_numerically(coeffs)
    assert len(roots) == 4, f"Expected 4 roots, got {len(roots)}"

    # Check conjugate pair structure
    geo = analyze_root_geometry(roots)
    assert geo["conjugate_pairs"] == 2, f"Expected 2 conjugate pairs"
    assert geo["has_real_axis_symmetry"], "Should have real-axis symmetry"


def test_classify_basin():
    roots = [Complex(1, 0), Complex(-1, 0), Complex(0, 1)]
    assert classify_basin(Complex(0.99, 0.01), roots, eps=0.05) == 0
    assert classify_basin(Complex(-0.98, 0.02), roots, eps=0.05) == 1
    assert classify_basin(Complex(0.01, 0.99), roots, eps=0.05) == 2
    assert classify_basin(Complex(5.0, 5.0), roots, eps=0.01) == -1


def test_basin_map_z3():
    """Basin map of z^3 - 1 should have 3 basins with roughly equal size."""
    coeffs = [-1.0, 0.0, 0.0, 1.0, 0.0]
    roots = [
        Complex(1.0, 0.0),
        Complex(-0.5, math.sqrt(3) / 2),
        Complex(-0.5, -math.sqrt(3) / 2),
    ]
    bm = compute_basin_map(coeffs, roots, (0.0, 0.0), 1.0, 64, 64, sample_step=2)
    assert bm["total_sampled_pixels"] > 0
    # Each basin should have >10% of pixels
    for b in bm["basins"]:
        assert b["fraction"] > 0.10, f"Basin {b['root_index']} too small: {b['fraction']}"


def test_root_geometry_square():
    """4 roots at ±1±i should have equal distances."""
    roots = [Complex(1, 1), Complex(1, -1), Complex(-1, 1), Complex(-1, -1)]
    geo = analyze_root_geometry(roots)
    assert geo["n_roots"] == 4
    assert abs(geo["centroid"]["x"]) < 1e-6
    assert abs(geo["centroid"]["y"]) < 1e-6
    assert abs(geo["radius_spread"]) < 1e-6, "All roots equidistant from origin"
    assert geo["conjugate_pairs"] == 2


def test_analyze_finding_roundtrip():
    """Create a minimal finding directory and analyze it."""
    with tempfile.TemporaryDirectory() as tmpdir:
        finding_dir = Path(tmpdir)

        state = {
            "state_version": 3,
            "fractal_type": "newton",
            "view": {"center_x": 0.0, "center_y": 0.0, "zoom": 1.0},
            "params": {
                "poly_coeffs": [-1.0, 0.0, 0.0, 1.0, 0.0],
                "explaino_root_count": 0,
            },
            "render": {"width": 64, "height": 64},
        }
        (finding_dir / "state.json").write_text(json.dumps(state))
        (finding_dir / "finding.json").write_text(json.dumps({"finding_id": "test_roundtrip"}))

        analysis = analyze_finding(finding_dir, sample_step=8)
        assert analysis.finding_id == "test_roundtrip"
        assert len(analysis.roots) == 3, f"z^3-1 has 3 roots, got {len(analysis.roots)}"
        assert analysis.basin_map["total_sampled_pixels"] > 0

        report = format_report(analysis)
        assert "test_roundtrip" in report
        assert "newton" in report


if __name__ == "__main__":
    tests = [
        test_poly_eval_z3_minus_1,
        test_newton_converges_z3,
        test_find_roots_z4_minus_1,
        test_find_roots_explaino_dual,
        test_classify_basin,
        test_basin_map_z3,
        test_root_geometry_square,
        test_analyze_finding_roundtrip,
    ]
    failures = 0
    for t in tests:
        try:
            t()
            print(f"  PASS: {t.__name__}")
        except Exception as e:
            print(f"  FAIL: {t.__name__}: {e}")
            failures += 1
    print(f"\n{len(tests) - failures}/{len(tests)} passed")
    sys.exit(1 if failures else 0)
