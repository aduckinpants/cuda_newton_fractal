#!/usr/bin/env python3
"""
Test coverage audit for the fractal viewer-host C++ codebase.

Maps source modules to test files, reports coverage gaps, and tracks
a coverage matrix inspired by nine repo's data_orientation.py.

Usage:
    python tools/test_coverage_audit.py                  # report gaps
    python tools/test_coverage_audit.py --matrix         # full coverage matrix
    python tools/test_coverage_audit.py --check-baseline # fail on coverage regression
"""

import argparse
import json
import re
import sys
from pathlib import Path

BASELINE_PATH = Path(__file__).parent / "test_coverage_baseline.json"


def discover_modules(repo_root: Path) -> dict[str, dict]:
    """Discover all source modules (each .cpp/.cu in src/ is a module)."""
    src_dir = repo_root / "ui_app" / "src"
    modules = {}
    
    for ext in ("*.cpp", "*.cu", "*.h"):
        for f in sorted(src_dir.glob(ext)):
            name = f.stem
            rel = f.relative_to(repo_root)
            
            # Classify module type
            if f.suffix == ".h":
                mod_type = "header-only"
            elif f.suffix == ".cu":
                mod_type = "cuda"
            else:
                mod_type = "cpp"
            
            # Skip third-party
            if "imgui" in str(f).lower():
                continue
            
            modules[name] = {
                "path": str(rel),
                "type": mod_type,
                "lines": len(f.read_text(encoding="utf-8", errors="replace").split("\n")),
            }
    
    return modules


def discover_tests(repo_root: Path) -> dict[str, dict]:
    """Discover all test files and count test functions."""
    test_dir = repo_root / "ui_app" / "tests"
    tests = {}
    
    if not test_dir.exists():
        return tests
    
    for f in sorted(test_dir.glob("test_*.cpp")):
        name = f.stem
        rel = f.relative_to(repo_root)
        text = f.read_text(encoding="utf-8", errors="replace")
        
        # Count test functions (bool Test... or RUN(Test... patterns)
        test_fns = re.findall(r"bool\s+(Test\w+)\s*\(", text)
        run_calls = re.findall(r"RUN\(\s*(\w+)", text)
        
        # Also count PASS/FAIL assertion macros
        assert_count = len(re.findall(r"\bASSERT\s*\(", text))
        check_count = len(re.findall(r"\bCheck\s*\(", text))
        
        tests[name] = {
            "path": str(rel),
            "test_functions": len(test_fns),
            "run_calls": len(run_calls),
            "assertions": assert_count + check_count,
            "lines": len(text.split("\n")),
            "functions": test_fns,
        }
    
    # Also check .cu test files
    for f in sorted(test_dir.glob("test_*.cu")):
        name = f.stem
        rel = f.relative_to(repo_root)
        text = f.read_text(encoding="utf-8", errors="replace")
        test_fns = re.findall(r"bool\s+(Test\w+)\s*\(", text)
        
        tests[name] = {
            "path": str(rel),
            "test_functions": len(test_fns),
            "run_calls": 0,
            "assertions": len(re.findall(r"\bASSERT\b|\bassert\b|\bASSERT_", text)),
            "lines": len(text.split("\n")),
            "functions": test_fns,
            "type": "cuda",
        }
    
    return tests


def map_coverage(modules: dict, tests: dict) -> list[dict]:
    """Map each source module to its test coverage status."""
    coverage = []
    
    # Build a lookup from test name -> module name
    # Convention: test_foo.cpp tests foo.cpp/foo.h
    test_targets = {}
    for test_name in tests:
        # Strip "test_" prefix
        target = test_name.removeprefix("test_")
        test_targets[target] = test_name
    
    for mod_name, mod_info in sorted(modules.items()):
        # Check if this module has a direct test
        test_name = test_targets.get(mod_name)
        
        # Also check if it's tested indirectly (included in another test's compile)
        indirect_tests = []
        if not test_name:
            # Check build script for indirect inclusion
            # (this is heuristic; the build script is the authority)
            for tn, ti in tests.items():
                # A test that has this module's name in its function list
                for fn in ti.get("functions", []):
                    if mod_name.replace("_", "") in fn.lower():
                        indirect_tests.append(tn)
                        break
        
        status = "COVERED" if test_name else ("INDIRECT" if indirect_tests else "UNCOVERED")
        
        entry = {
            "module": mod_name,
            "path": mod_info["path"],
            "type": mod_info["type"],
            "lines": mod_info["lines"],
            "status": status,
            "test": test_name,
            "indirect_tests": indirect_tests,
        }
        
        if test_name and test_name in tests:
            ti = tests[test_name]
            entry["test_functions"] = ti["test_functions"]
            entry["assertions"] = ti["assertions"]
        
        coverage.append(entry)
    
    return coverage


def classify_by_family(mod_name: str) -> str:
    """Classify modules by functional family (inspired by nine conftest markers)."""
    families = {
        "explaino": ["explaino_seed", "explaino_seed_dynamics", "explaino_collatz"],
        "view": ["view_hp_sync", "viewport_interaction"],
        "schema": ["ui_schema", "schema_binding", "schema_startup_policy", "safe_mode_schema"],
        "probe": ["fractal_probe_runner", "fractal_probe_contract", "function_descriptor"],
        "render": ["fractal_renderer", "sample_tier_resolver"],
        "diagnostics": ["diagnostics_state_io", "diagnostics_capture", "finding_state_actions",
                        "finding_archive_actions", "render_capture_guard"],
        "sweep": ["sweep_player", "viewer_sweep"],
        "viewer": ["viewer_shutdown", "viewer_render_pacing", "viewer_sweep"],
        "kernel_math": ["basin_coloring", "escape_time_coloring", "escape_time_direct_formulas",
                        "escape_time_specialized_formulas", "polynomial_eval_real_coeffs",
                        "perturbation_reference_orbit"],
        "io": ["json_min", "cli_args", "headless_modes"],
        "types": ["fractal_types", "fractal_family_rules", "fractal_derived_fields"],
    }
    
    for family, members in families.items():
        if mod_name in members:
            return family
    return "other"


def print_report(coverage: list[dict], tests: dict):
    """Print human-readable coverage report."""
    covered = [c for c in coverage if c["status"] == "COVERED"]
    indirect = [c for c in coverage if c["status"] == "INDIRECT"]
    uncovered = [c for c in coverage if c["status"] == "UNCOVERED"]
    
    total_assertions = sum(t.get("assertions", 0) for t in tests.values())
    total_test_fns = sum(t.get("test_functions", 0) + t.get("run_calls", 0) for t in tests.values())
    
    print(f"\n=== Test Coverage Audit ===")
    print(f"Source modules: {len(coverage)}")
    print(f"  COVERED:   {len(covered):3d} (direct test file)")
    print(f"  INDIRECT:  {len(indirect):3d} (tested via other tests)")
    print(f"  UNCOVERED: {len(uncovered):3d}")
    print(f"")
    print(f"Test files:      {len(tests)}")
    print(f"Test functions:  {total_test_fns}")
    print(f"Assertions:      {total_assertions}")
    
    if uncovered:
        print(f"\nUNCOVERED modules:")
        for c in sorted(uncovered, key=lambda x: -x["lines"]):
            family = classify_by_family(c["module"])
            print(f"  {c['lines']:5d} lines  [{family:12s}]  {c['path']}")
    
    if indirect:
        print(f"\nINDIRECT coverage (no dedicated test file):")
        for c in sorted(indirect, key=lambda x: -x["lines"]):
            family = classify_by_family(c["module"])
            via = ", ".join(c["indirect_tests"][:3])
            print(f"  {c['lines']:5d} lines  [{family:12s}]  {c['path']}  (via {via})")


def print_matrix(coverage: list[dict]):
    """Print coverage matrix grouped by family."""
    from collections import defaultdict
    families = defaultdict(list)
    
    for c in coverage:
        family = classify_by_family(c["module"])
        families[family].append(c)
    
    print(f"\n=== Coverage Matrix by Family ===")
    for family in sorted(families.keys()):
        members = families[family]
        covered_pct = sum(1 for m in members if m["status"] != "UNCOVERED") / max(len(members), 1) * 100
        print(f"\n{family} ({covered_pct:.0f}% covered):")
        for m in sorted(members, key=lambda x: x["module"]):
            status_char = {"COVERED": "+", "INDIRECT": "~", "UNCOVERED": "-"}[m["status"]]
            extra = ""
            if m["status"] == "COVERED" and "test_functions" in m:
                extra = f"  ({m['test_functions']} tests, {m.get('assertions', 0)} assertions)"
            print(f"  [{status_char}] {m['module']}{extra}")


def check_baseline(coverage: list[dict], baseline_path: Path) -> bool:
    """Check against baseline. Fail if any previously-covered module loses coverage."""
    if not baseline_path.exists():
        print(f"No baseline at {baseline_path}; skipping regression check.")
        return True
    
    baseline = json.loads(baseline_path.read_text())
    baseline_covered = set(baseline.get("covered_modules", []))
    
    current_covered = set(c["module"] for c in coverage if c["status"] == "COVERED")
    
    lost = baseline_covered - current_covered
    if lost:
        print(f"COVERAGE REGRESSION: lost direct coverage for: {', '.join(sorted(lost))}")
        return False
    
    gained = current_covered - baseline_covered
    if gained:
        print(f"  New coverage: {', '.join(sorted(gained))}")
    
    return True


def update_baseline(coverage: list[dict], baseline_path: Path):
    """Write current coverage as baseline."""
    baseline = {
        "covered_modules": sorted(c["module"] for c in coverage if c["status"] == "COVERED"),
        "total_modules": len(coverage),
        "updated": __import__("datetime").datetime.now().isoformat()[:10],
    }
    baseline_path.write_text(json.dumps(baseline, indent=2) + "\n")
    print(f"Baseline updated: {len(baseline['covered_modules'])} covered modules")


def main():
    parser = argparse.ArgumentParser(description="C++ test coverage audit")
    parser.add_argument("--matrix", action="store_true", help="Print family coverage matrix")
    parser.add_argument("--check-baseline", action="store_true", help="Fail on coverage regression")
    parser.add_argument("--update-baseline", action="store_true", help="Lock current coverage as baseline")
    parser.add_argument("--out", type=str, default=None, help="Write JSON report")
    parser.add_argument("--repo-root", type=str, default=None)
    args = parser.parse_args()
    
    repo_root = Path(args.repo_root) if args.repo_root else Path(__file__).parent.parent
    
    modules = discover_modules(repo_root)
    tests = discover_tests(repo_root)
    coverage = map_coverage(modules, tests)
    
    print_report(coverage, tests)
    
    if args.matrix:
        print_matrix(coverage)
    
    if args.out:
        out_path = Path(args.out)
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(json.dumps({
            "coverage": coverage,
            "tests": {k: {kk: vv for kk, vv in v.items() if kk != "functions"} for k, v in tests.items()},
        }, indent=2) + "\n")
        print(f"\nJSON report written to {args.out}")
    
    if args.update_baseline:
        update_baseline(coverage, BASELINE_PATH)
    
    if args.check_baseline:
        if not check_baseline(coverage, BASELINE_PATH):
            sys.exit(1)
        print("Baseline check passed.")


if __name__ == "__main__":
    main()
