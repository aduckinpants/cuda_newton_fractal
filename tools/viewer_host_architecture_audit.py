from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Any

REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_OUT = REPO_ROOT / "artifacts" / "architecture" / "viewer_host_architecture_audit.json"
DEFAULT_MAINLINE_ROOT = Path(r"C:\code\salticid-cuda")

RISK_SURFACES = {
    "viewer_shell": "ui_app/src/main.cpp",
    "color_pipeline_window": "ui_app/src/color_pipeline_window.h",
    "schema_binding": "ui_app/src/schema_binding.cpp",
    "schema_loader": "ui_app/src/ui_schema.cpp",
    "fractal_family_rules": "ui_app/src/fractal_family_rules.h",
    "runtime_walk_viewer_pytest": "tests/test_fractal_runtime_runtime_walk_viewer.py",
    "runtime_harness": "tests/runtime_harness.py",
}

LINE_BASELINES = {
    "ui_app/src/main.cpp": 2930,
    "ui_app/src/color_pipeline_window.h": 4221,
    "ui_app/src/schema_binding.cpp": 1818,
    "ui_app/src/ui_schema.cpp": 240,
    "ui_app/src/fractal_family_rules.h": 985,
    "tests/test_fractal_runtime_runtime_walk_viewer.py": 1294,
    "tests/runtime_harness.py": 333,
}

FUNCTION_BASELINES = {
    "ui_app/src/main.cpp": 44,
    "ui_app/src/color_pipeline_window.h": 93,
    "ui_app/src/schema_binding.cpp": 70,
    "ui_app/src/ui_schema.cpp": 7,
    "ui_app/src/fractal_family_rules.h": 57,
}

FUNCTION_RE = re.compile(
    r"^\s*(?:static\s+|inline\s+|constexpr\s+|FRACTAL_FAMILY_RULES_HD\s+)*"
    r"(?:bool|void|int|double|float|std::[\w:<>]+|[A-Z][\w:<>]+)\s+"
    r"[A-Za-z_]\w*\s*\([^;]*\)\s*(?:\{|$)"
)


def count_functions(text: str) -> int:
    return sum(1 for line in text.splitlines() if FUNCTION_RE.match(line))


def file_metrics(rel_path: str) -> dict[str, Any]:
    path = REPO_ROOT / rel_path
    text = path.read_text(encoding="utf-8", errors="replace")
    return {
        "path": rel_path,
        "exists": path.exists(),
        "line_count": len(text.splitlines()),
        "function_count": count_functions(text),
    }


def build_mainline_tool_status(mainline_root: Path) -> dict[str, Any]:
    architecture_audit = mainline_root / "tools" / "native_architecture_audit.py"
    cppdepend_status = mainline_root / "tools" / "native_analysis" / "cppdepend_status.py"
    status: dict[str, Any] = {
        "mainline_root": str(mainline_root),
        "native_architecture_audit_present": architecture_audit.exists(),
        "cppdepend_status_present": cppdepend_status.exists(),
        "repo_compile_commands_present": (REPO_ROOT / "compile_commands.json").exists(),
        "ready_for_cppdepend_console": False,
        "notes": [],
    }
    if architecture_audit.exists():
        text = architecture_audit.read_text(encoding="utf-8", errors="replace")
        status["native_architecture_audit_native_src_bound"] = "native/src" in text
        if "native/src" in text:
            status["notes"].append("mainline native architecture audit is useful as a pattern but is not a drop-in viewer-host scan")
    if not status["repo_compile_commands_present"]:
        status["notes"].append("viewer-host repo has no repo-root compile_commands.json yet, so CppDepend console analysis is not ready")
    status["ready_for_cppdepend_console"] = bool(status["repo_compile_commands_present"] and cppdepend_status.exists())
    return status


def build_report(mainline_root: Path = DEFAULT_MAINLINE_ROOT) -> dict[str, Any]:
    metrics = [file_metrics(path) for path in RISK_SURFACES.values()]
    findings: list[dict[str, Any]] = []
    for row in metrics:
        path = str(row["path"])
        line_baseline = LINE_BASELINES.get(path)
        if line_baseline is not None and int(row["line_count"]) > line_baseline:
            findings.append({
                "severity": "error",
                "code": "risk_surface_line_growth",
                "path": path,
                "message": f"{path} grew above baseline {line_baseline} lines",
                "actual": row["line_count"],
                "baseline": line_baseline,
            })
        fn_baseline = FUNCTION_BASELINES.get(path)
        if fn_baseline is not None and int(row["function_count"]) > fn_baseline:
            findings.append({
                "severity": "error",
                "code": "risk_surface_function_growth",
                "path": path,
                "message": f"{path} grew above baseline {fn_baseline} functions",
                "actual": row["function_count"],
                "baseline": fn_baseline,
            })
    mainline = build_mainline_tool_status(mainline_root)
    if not mainline["ready_for_cppdepend_console"]:
        findings.append({
            "severity": "warn",
            "code": "cppdepend_not_ready",
            "path": "compile_commands.json",
            "message": "CppDepend/NDepend-style console analysis is not ready for this repo until a repo-root compile database exists",
        })
    return {
        "schema_version": "viewer_host_architecture_audit.v1",
        "ok": not any(row["severity"] == "error" for row in findings),
        "repo_root": str(REPO_ROOT),
        "risk_surfaces": metrics,
        "line_baselines": LINE_BASELINES,
        "function_baselines": FUNCTION_BASELINES,
        "mainline_tool_status": mainline,
        "findings": findings,
    }


def write_report(report: dict[str, Any], out_path: Path) -> None:
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Audit viewer-host architectural risk surfaces and structural-analysis readiness.")
    parser.add_argument("--out", default=str(DEFAULT_OUT))
    parser.add_argument("--mainline-root", default=str(DEFAULT_MAINLINE_ROOT))
    parser.add_argument("--check-baseline", action="store_true")
    args = parser.parse_args(argv)

    report = build_report(Path(args.mainline_root))
    write_report(report, Path(args.out))
    print(json.dumps({
        "ok": report["ok"],
        "findings": len(report["findings"]),
        "out": str(Path(args.out)),
    }, indent=2))
    return 0 if (report["ok"] or not args.check_baseline) else 1


if __name__ == "__main__":
    raise SystemExit(main())
