from __future__ import annotations

import csv
import json
from datetime import datetime
from pathlib import Path
from typing import Any, Callable, Sequence

from .paths import publish_root
from .probe_client import describe_functions, run_sample_request


DEFAULT_SMOKE_POINTS = [
    {"x": 0.0, "y": 0.0},
    {"x": 0.25, "y": 0.0},
]

DEFAULT_SMOKE_METRICS = [
    "iterations",
    "status",
    "summary_mean_iterations",
    "summary_escape_fraction",
    "summary_converged_fraction",
]


def extract_fractal_types(catalog: dict[str, Any], function_id: str = "fractal.sample") -> list[str]:
    functions = catalog.get("functions")
    if not isinstance(functions, list):
        raise ValueError("describe-functions catalog must contain a functions array")

    for function in functions:
        if not isinstance(function, dict) or function.get("id") != function_id:
            continue
        parameters = function.get("parameters")
        if not isinstance(parameters, list):
            break
        for parameter in parameters:
            if not isinstance(parameter, dict):
                continue
            if parameter.get("path") != "fractal.view.fractal_type":
                continue
            options = parameter.get("options")
            if not isinstance(options, list):
                raise ValueError("fractal.view.fractal_type options must be a list")

            ids: list[str] = []
            for option in options:
                if isinstance(option, str):
                    ids.append(option)
                    continue
                if isinstance(option, dict) and isinstance(option.get("id"), str):
                    ids.append(option["id"])
                    continue
                raise ValueError("fractal.view.fractal_type options must be strings or {id,label} objects")
            return ids

    raise ValueError(f"No fractal.view.fractal_type enum options found for function_id={function_id}")


def build_smoke_request(
    fractal_type: str,
    *,
    points: Sequence[dict[str, float]] | None = None,
    metrics: Sequence[str] | None = None,
) -> dict[str, object]:
    return {
        "request_version": 1,
        "request_id": f"catalog-smoke-{fractal_type}",
        "function_id": "fractal.sample",
        "mode": "point_set",
        "overrides": [
            {"path": "fractal.view.fractal_type", "value": fractal_type},
        ],
        "points": list(points or DEFAULT_SMOKE_POINTS),
        "metrics": list(metrics or DEFAULT_SMOKE_METRICS),
        "operator_context": {
            "source": "reality_toolkit",
            "operator": "fractal_catalog_smoke",
            "why": f"Smoke sample for advertised fractal type {fractal_type}",
        },
    }


def run_fractal_catalog_smoke(
    *,
    repo_root: Path,
    function_id: str = "fractal.sample",
    points: Sequence[dict[str, float]] | None = None,
    metrics: Sequence[str] | None = None,
    exe_path: Path | None = None,
    timeout_seconds: float = 60.0,
    describe_runner: Callable[..., dict[str, Any]] = describe_functions,
    sample_runner: Callable[..., dict[str, Any]] = run_sample_request,
) -> dict[str, Any]:
    catalog = describe_runner(repo_root, exe_path=exe_path, timeout_seconds=timeout_seconds)
    fractal_types = extract_fractal_types(catalog, function_id=function_id)

    results: list[dict[str, Any]] = []
    for fractal_type in fractal_types:
        request = build_smoke_request(fractal_type, points=points, metrics=metrics)
        try:
            response = sample_runner(repo_root, request, exe_path=exe_path, timeout_seconds=timeout_seconds)
            summary = response.get("summary") if isinstance(response, dict) else None
            runtime = response.get("runtime") if isinstance(response, dict) else None
            results.append({
                "fractal_type": fractal_type,
                "ok": True,
                "runtime_fractal_type": runtime.get("fractal_type") if isinstance(runtime, dict) else None,
                "summary": summary if isinstance(summary, dict) else None,
                "error": None,
            })
        except Exception as exc:
            results.append({
                "fractal_type": fractal_type,
                "ok": False,
                "runtime_fractal_type": None,
                "summary": None,
                "error": str(exc),
            })

    ok_count = sum(1 for result in results if result["ok"])
    report = {
        "timestamp": datetime.now().isoformat(),
        "function_id": function_id,
        "fractal_types_total": len(fractal_types),
        "fractal_types_ok": ok_count,
        "fractal_types_failed": len(fractal_types) - ok_count,
        "points": list(points or DEFAULT_SMOKE_POINTS),
        "metrics": list(metrics or DEFAULT_SMOKE_METRICS),
        "results": results,
    }
    return report


def write_fractal_catalog_smoke_report(out_dir: Path, report: dict[str, Any]) -> tuple[Path, Path]:
    out_dir.mkdir(parents=True, exist_ok=True)

    json_path = out_dir / "fractal_catalog_smoke.json"
    json_path.write_text(json.dumps(report, indent=2), encoding="utf-8")

    csv_path = out_dir / "fractal_catalog_smoke.csv"
    with csv_path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle)
        writer.writerow([
            "fractal_type",
            "ok",
            "runtime_fractal_type",
            "sample_count",
            "mean_iterations",
            "escape_fraction",
            "converged_fraction",
            "error",
        ])
        for result in report.get("results", []):
            summary = result.get("summary") if isinstance(result, dict) else None
            summary = summary if isinstance(summary, dict) else {}
            writer.writerow([
                result.get("fractal_type") if isinstance(result, dict) else None,
                result.get("ok") if isinstance(result, dict) else None,
                result.get("runtime_fractal_type") if isinstance(result, dict) else None,
                summary.get("sample_count", ""),
                summary.get("mean_iterations", ""),
                summary.get("escape_fraction", ""),
                summary.get("converged_fraction", ""),
                result.get("error") if isinstance(result, dict) else None,
            ])
    return json_path, csv_path


def default_fractal_catalog_smoke_out_dir() -> Path:
    stamp = datetime.now().strftime("%Y-%m-%d_%H%M%S")
    return publish_root() / "artifacts" / f"fractal_catalog_smoke_{stamp}"
