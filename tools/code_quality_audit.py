#!/usr/bin/env python3
"""
Code quality audit for the fractal viewer-host C++ codebase.

Adapted from the mainline salticid-cuda three-pillar quality system.
Measures function sizes, detects structural issues, and tracks a ratchet
baseline so regressions are caught automatically.

Usage:
    python tools/code_quality_audit.py                     # report only
    python tools/code_quality_audit.py --check-baseline    # fail on regression
    python tools/code_quality_audit.py --update-baseline   # lock current score
    python tools/code_quality_audit.py --out report.json   # write JSON report
"""

import argparse
import json
import os
import re
import sys
from pathlib import Path

# --- Thresholds (aligned with mainline) ---
CRITICAL_THRESHOLD = 500   # >= 500 lines: CRITICAL (C++ tends smaller than Python)
ERROR_THRESHOLD    = 200   # >= 200 lines: ERROR
WARN_THRESHOLD     = 80    # >= 80 lines: WARN
NOTE_THRESHOLD     = 50    # >= 50 lines: NOTE

# Scoring: 100 - 20*critical - 5*error - 1*warn, clamped [0, 100]
CRITICAL_PENALTY = 20
ERROR_PENALTY    = 5
WARN_PENALTY     = 1

# Globs for C++ source files
SOURCE_GLOBS = [
    "ui_app/src/*.cpp",
    "ui_app/src/*.cu",
    "ui_app/src/*.h",
]

BASELINE_PATH = Path(__file__).parent / "code_quality_baseline.json"


def find_functions(filepath: Path) -> list[dict]:
    """Extract function definitions with line counts from a C/C++/CUDA file."""
    try:
        text = filepath.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return []

    lines = text.split("\n")
    functions = []
    
    # Simple brace-counting parser.
    # Detects function definitions by looking for `name(...)` followed by `{`
    # at the top level (brace depth 0).
    func_pattern = re.compile(
        r"^(?!.*(?:if|else|for|while|switch|namespace|class|struct|enum|catch|do)\b)"
        r".*?(\b\w+)\s*\([^;]*\)\s*(?:const)?\s*(?:override)?\s*\{?\s*$"
    )
    
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        
        # Skip preprocessor, comments, blank lines at scan level
        if not line or line.startswith("//") or line.startswith("#") or line.startswith("/*"):
            i += 1
            continue
        
        # Look for function-like definitions
        # Heuristic: line has identifier(...) possibly followed by { on same or next line
        m = func_pattern.match(line)
        if m:
            func_name = m.group(1)
            # Find the opening brace
            brace_line = i
            if "{" not in lines[i]:
                # Might be on next non-blank line
                j = i + 1
                while j < len(lines) and not lines[j].strip():
                    j += 1
                if j < len(lines) and "{" in lines[j]:
                    brace_line = j
                else:
                    i += 1
                    continue
            
            # Count lines until matching close brace
            depth = 0
            start_line = brace_line
            found_open = False
            for k in range(brace_line, len(lines)):
                for ch in lines[k]:
                    if ch == "{":
                        depth += 1
                        found_open = True
                    elif ch == "}":
                        depth -= 1
                        if found_open and depth == 0:
                            end_line = k
                            length = end_line - start_line + 1
                            functions.append({
                                "name": func_name,
                                "file": str(filepath),
                                "start_line": start_line + 1,
                                "end_line": end_line + 1,
                                "lines": length,
                            })
                            i = end_line + 1
                            break
                else:
                    continue
                break
            else:
                i += 1
        else:
            i += 1
    
    return functions


def classify(lines: int) -> str:
    if lines >= CRITICAL_THRESHOLD:
        return "CRITICAL"
    if lines >= ERROR_THRESHOLD:
        return "ERROR"
    if lines >= WARN_THRESHOLD:
        return "WARN"
    if lines >= NOTE_THRESHOLD:
        return "NOTE"
    return "OK"


def compute_score(functions: list[dict]) -> int:
    critical = sum(1 for f in functions if f["severity"] == "CRITICAL")
    error = sum(1 for f in functions if f["severity"] == "ERROR")
    warn = sum(1 for f in functions if f["severity"] == "WARN")
    raw = 100 - CRITICAL_PENALTY * critical - ERROR_PENALTY * error - WARN_PENALTY * warn
    return max(0, min(100, raw))


def scan_magic_numbers(filepath: Path) -> list[dict]:
    """Detect suspicious hardcoded numeric literals in source code."""
    try:
        text = filepath.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return []

    issues = []
    safe_numbers = {"0", "1", "2", "-1", "0.0", "1.0", "2.0", "-1.0",
                    "0.0f", "1.0f", "2.0f", "-1.0f", "0.5f", "0.5",
                    "3.14159", "3.14159265358979", "255", "256", "1024", "768"}
    
    # Match floating point with many digits (likely magic number)
    magic_re = re.compile(r"(?<![\"'\w])(\d+\.\d{3,}f?)(?![\"'\w])")
    
    for i, line in enumerate(text.split("\n"), 1):
        stripped = line.strip()
        # Skip comments, includes, constexpr/const definitions
        if stripped.startswith("//") or stripped.startswith("#") or stripped.startswith("/*"):
            continue
        if "constexpr" in stripped or "const " in stripped:
            continue
            
        for m in magic_re.finditer(line):
            val = m.group(1)
            if val not in safe_numbers:
                issues.append({
                    "file": str(filepath),
                    "line": i,
                    "value": val,
                    "context": stripped[:80],
                })
    
    return issues


def find_dead_statics(filepath: Path) -> list[dict]:
    """Find static functions that might be unused (declared but never called)."""
    try:
        text = filepath.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return []
    
    # Find all static function declarations
    static_funcs = re.findall(r"^static\s+(?:inline\s+)?(?:\w+\s+)+(\w+)\s*\(", text, re.MULTILINE)
    
    issues = []
    for name in static_funcs:
        # Count occurrences beyond the definition
        count = len(re.findall(r"\b" + re.escape(name) + r"\b", text))
        if count <= 1:
            issues.append({
                "file": str(filepath),
                "name": name,
                "references": count,
            })
    
    return issues


def run_audit(repo_root: Path) -> dict:
    """Run full audit and return structured report."""
    all_functions = []
    all_magic = []
    all_dead = []
    file_stats = {}
    
    for glob in SOURCE_GLOBS:
        for filepath in sorted(repo_root.glob(glob)):
            rel = filepath.relative_to(repo_root)
            
            funcs = find_functions(filepath)
            for f in funcs:
                f["severity"] = classify(f["lines"])
                f["file"] = str(rel)
            all_functions.extend(funcs)
            
            magic = scan_magic_numbers(filepath)
            for m in magic:
                m["file"] = str(rel)
            all_magic.extend(magic)
            
            dead = find_dead_statics(filepath)
            for d in dead:
                d["file"] = str(rel)
            all_dead.extend(dead)
            
            try:
                line_count = len(filepath.read_text(encoding="utf-8", errors="replace").split("\n"))
            except OSError:
                line_count = 0
            
            file_stats[str(rel)] = {
                "lines": line_count,
                "function_count": len(funcs),
                "max_fn_lines": max((f["lines"] for f in funcs), default=0),
            }
    
    score = compute_score(all_functions)
    
    report = {
        "score": score,
        "total_functions": len(all_functions),
        "severity_counts": {
            "CRITICAL": sum(1 for f in all_functions if f["severity"] == "CRITICAL"),
            "ERROR": sum(1 for f in all_functions if f["severity"] == "ERROR"),
            "WARN": sum(1 for f in all_functions if f["severity"] == "WARN"),
            "NOTE": sum(1 for f in all_functions if f["severity"] == "NOTE"),
            "OK": sum(1 for f in all_functions if f["severity"] == "OK"),
        },
        "functions": sorted(all_functions, key=lambda f: -f["lines"]),
        "magic_numbers": all_magic[:50],  # cap for readability
        "dead_statics": all_dead,
        "file_stats": file_stats,
    }
    
    return report


def check_baseline(report: dict, baseline_path: Path) -> bool:
    """Compare report against baseline. Return True if no regression."""
    if not baseline_path.exists():
        print(f"No baseline at {baseline_path}; skipping regression check.")
        return True
    
    baseline = json.loads(baseline_path.read_text())
    ok = True
    
    # Score regression check (allow 2-point tolerance)
    if report["score"] < baseline.get("score", 0) - 2:
        print(f"REGRESSION: score {report['score']} < baseline {baseline['score']} (tolerance 2)")
        ok = False
    
    # Per-file max_fn_lines regression
    baseline_files = baseline.get("file_stats", {})
    for fpath, stats in report["file_stats"].items():
        if fpath in baseline_files:
            old_max = baseline_files[fpath].get("max_fn_lines", 0)
            new_max = stats["max_fn_lines"]
            if new_max > old_max and old_max > 0:
                print(f"REGRESSION: {fpath} max_fn_lines {old_max} -> {new_max}")
                ok = False
    
    return ok


def update_baseline(report: dict, baseline_path: Path):
    """Write current report as the new baseline."""
    baseline = {
        "score": report["score"],
        "file_stats": report["file_stats"],
        "updated": __import__("datetime").datetime.now().isoformat()[:10],
    }
    baseline_path.write_text(json.dumps(baseline, indent=2) + "\n")
    print(f"Baseline updated: score={report['score']}, written to {baseline_path}")


def print_report(report: dict):
    """Print human-readable summary."""
    print(f"\n=== Code Quality Audit ===")
    print(f"Score: {report['score']}/100")
    print(f"Functions scanned: {report['total_functions']}")
    
    sc = report["severity_counts"]
    print(f"  CRITICAL: {sc['CRITICAL']}  ERROR: {sc['ERROR']}  WARN: {sc['WARN']}  NOTE: {sc['NOTE']}  OK: {sc['OK']}")
    
    # Show top offenders
    top = [f for f in report["functions"] if f["severity"] in ("CRITICAL", "ERROR", "WARN")]
    if top:
        print(f"\nTop offenders:")
        for f in top[:15]:
            print(f"  [{f['severity']:8s}] {f['file']}:{f['start_line']}  {f['name']}() — {f['lines']} lines")
    
    if report["magic_numbers"]:
        print(f"\nMagic numbers found: {len(report['magic_numbers'])}")
        for m in report["magic_numbers"][:10]:
            print(f"  {m['file']}:{m['line']}  {m['value']}  ({m['context']})")
    
    if report["dead_statics"]:
        print(f"\nPotentially dead static functions: {len(report['dead_statics'])}")
        for d in report["dead_statics"][:10]:
            print(f"  {d['file']}  {d['name']}() — {d['references']} reference(s)")
    
    # File size summary
    print(f"\nFile sizes (top 10):")
    sorted_files = sorted(report["file_stats"].items(), key=lambda kv: -kv[1]["lines"])
    for fpath, stats in sorted_files[:10]:
        print(f"  {stats['lines']:5d} lines  {stats['function_count']:3d} funcs  max={stats['max_fn_lines']:4d}  {fpath}")


def main():
    parser = argparse.ArgumentParser(description="C++ code quality audit with ratchet baseline")
    parser.add_argument("--check-baseline", action="store_true",
                        help="Fail if quality regressed vs baseline")
    parser.add_argument("--update-baseline", action="store_true",
                        help="Write current score as new baseline")
    parser.add_argument("--out", type=str, default=None,
                        help="Write JSON report to this path")
    parser.add_argument("--repo-root", type=str, default=None,
                        help="Repository root (default: auto-detect)")
    args = parser.parse_args()
    
    if args.repo_root:
        repo_root = Path(args.repo_root)
    else:
        repo_root = Path(__file__).parent.parent
    
    report = run_audit(repo_root)
    print_report(report)
    
    if args.out:
        out_path = Path(args.out)
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(json.dumps(report, indent=2) + "\n")
        print(f"\nJSON report written to {args.out}")
    
    if args.update_baseline:
        update_baseline(report, BASELINE_PATH)
    
    if args.check_baseline:
        if not check_baseline(report, BASELINE_PATH):
            print("\nBASELINE CHECK FAILED")
            sys.exit(1)
        else:
            print("\nBaseline check passed.")
    
    sys.exit(0 if report["severity_counts"]["CRITICAL"] == 0 else 1)


if __name__ == "__main__":
    main()
