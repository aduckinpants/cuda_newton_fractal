from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[1]
BUILD_SCRIPT = REPO_ROOT / "ui_app" / "build_tests_vsdevcmd.cmd"
REQUIRED_FOCUSED_TARGETS = (
    "test_viewer_render_pacing",
    "test_sample_tier_resolver",
    "test_fractal_renderer",
)
REQUIRED_MEASUREMENTS = {
    "pacing": Path("artifacts/validation/native_helper_measure_pacing_target.json"),
    "sample_tier": Path("artifacts/validation/native_helper_measure_sample_tier_target.json"),
    "fractal_renderer": Path("artifacts/validation/native_helper_measure_fractal_renderer_target.json"),
    "full_helper_baseline": Path("artifacts/validation/native_helper_full_measured.json"),
    "full_helper_optimized": Path("artifacts/validation/native_helper_full_optimized_measured.json"),
}
REQUIRED_GROUPED_PROOFS = {
    "multi_target_non_cuda": Path("artifacts/validation/native_helper_multi_target_non_cuda_after_audit.json"),
    "fractal_cuda_shared_objects": Path("artifacts/validation/native_helper_fractal_cuda_shared_objects.json"),
    "generic_cuda_shared_object": Path("artifacts/validation/native_helper_generic_cuda_shared_object.json"),
}
REQUIRED_NEGATIVE_PROOFS = {
    "unknown_target_rejected": Path("artifacts/validation/native_helper_unknown_target_negative.json"),
}
CUDA_SOURCE_NAMES = (
    "fractal_renderer.cu",
    "fractal_sample_core.cu",
    "generic_sample_core.cu",
)
CUDA_ARCH_FLAGS = (
    "-gencode=arch=compute_86,code=sm_86",
    "-gencode=arch=compute_120,code=sm_120",
    "-gencode=arch=compute_121,code=sm_121",
)


def _repo_path(path: Path) -> Path:
    return path if path.is_absolute() else REPO_ROOT / path


def _load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def _target_presence(script_text: str, target: str) -> dict[str, bool]:
    return {
        "dispatch": f'if /I "%FOCUSED_TEST%"=="{target}"' in script_text,
        "label": f":focused_{target}" in script_text,
    }


def _json_status(paths: dict[str, Path]) -> dict[str, dict[str, Any]]:
    out: dict[str, dict[str, Any]] = {}
    for name, rel_path in paths.items():
        path = _repo_path(rel_path)
        entry: dict[str, Any] = {"path": rel_path.as_posix(), "exists": path.exists()}
        if path.exists():
            payload = _load_json(path)
            entry.update(
                {
                    "ok": bool(payload.get("ok")),
                    "result": payload.get("result"),
                    "exit_code": payload.get("exit_code"),
                    "elapsed_seconds": payload.get("elapsed_seconds"),
                    "timed_out": payload.get("timed_out"),
                    "log": payload.get("log"),
                }
            )
        out[name] = entry
    return out


def _measurement_status() -> dict[str, dict[str, Any]]:
    return _json_status(REQUIRED_MEASUREMENTS)


def _grouped_proof_status() -> dict[str, dict[str, Any]]:
    return _json_status(REQUIRED_GROUPED_PROOFS)


def _negative_proof_status() -> dict[str, dict[str, Any]]:
    return _json_status(REQUIRED_NEGATIVE_PROOFS)


def _cuda_compile_profile(script_text: str) -> dict[str, Any]:
    source_counts = {name: script_text.count(f".\\src\\{name}") for name in CUDA_SOURCE_NAMES}
    arch_counts = {arch: script_text.count(arch) for arch in CUDA_ARCH_FLAGS}
    repeated_fractal_combo = (
        ".\\src\\fractal_renderer.cu .\\src\\fractal_sample_core.cu "
        ".\\src\\sample_tier_resolver.cpp .\\tests"
    )
    repeated_generic_combo = ".\\src\\generic_sample_core.cu .\\tests"
    return {
        "nvcc_invocation_count": script_text.count("nvcc -allow-unsupported-compiler"),
        "cuda_source_counts": source_counts,
        "cuda_arch_counts": arch_counts,
        "multi_target_focused_dispatch": (
            ":focused_arg_loop" in script_text
            and 'call :dispatch_focused "%~1"' in script_text
            and "shift" in script_text
        ),
        "shared_fractal_cuda_object_helper": ":build_fractal_cuda_common_objects" in script_text,
        "shared_generic_sample_core_object_helper": ":build_generic_sample_core_object" in script_text,
        "repeated_fractal_source_combo_count": script_text.count(repeated_fractal_combo),
        "repeated_generic_source_combo_count": script_text.count(repeated_generic_combo),
        "stale_errorlevel_dispatch_count": script_text.count(" & exit /b %ERRORLEVEL%"),
    }


def _performance_summary(measurements: dict[str, dict[str, Any]]) -> dict[str, Any]:
    baseline = measurements.get("full_helper_baseline", {})
    optimized = measurements.get("full_helper_optimized", {})
    baseline_seconds = baseline.get("elapsed_seconds")
    optimized_seconds = optimized.get("elapsed_seconds")
    improved = False
    speedup_percent = None
    if isinstance(baseline_seconds, (int, float)) and isinstance(optimized_seconds, (int, float)):
        improved = optimized_seconds < baseline_seconds
        if baseline_seconds > 0:
            speedup_percent = ((baseline_seconds - optimized_seconds) / baseline_seconds) * 100.0
    return {
        "baseline_seconds": baseline_seconds,
        "optimized_seconds": optimized_seconds,
        "improved": improved,
        "speedup_percent": speedup_percent,
    }


def build_report() -> dict[str, Any]:
    script_text = BUILD_SCRIPT.read_text(encoding="utf-8")
    focused_targets = {target: _target_presence(script_text, target) for target in REQUIRED_FOCUSED_TARGETS}
    measurements = _measurement_status()
    grouped_proofs = _grouped_proof_status()
    negative_proofs = _negative_proof_status()
    performance_summary = _performance_summary(measurements)
    compile_profile = _cuda_compile_profile(script_text)
    optimized_topology = (
        compile_profile["multi_target_focused_dispatch"]
        and compile_profile["shared_fractal_cuda_object_helper"]
        and compile_profile["shared_generic_sample_core_object_helper"]
        and compile_profile["repeated_fractal_source_combo_count"] == 0
        and compile_profile["repeated_generic_source_combo_count"] == 0
        and compile_profile["stale_errorlevel_dispatch_count"] == 0
        and all(count == 1 for count in compile_profile["cuda_arch_counts"].values())
    )
    checks = {
        "focused_targets_present": all(item["dispatch"] and item["label"] for item in focused_targets.values()),
        "measurement_jsons_present": all(item.get("exists") for item in measurements.values()),
        "measurement_jsons_success": all(item.get("ok") and item.get("exit_code") == 0 for item in measurements.values()),
        "grouped_proofs_present": all(item.get("exists") for item in grouped_proofs.values()),
        "grouped_proofs_success": all(item.get("ok") and item.get("exit_code") == 0 for item in grouped_proofs.values()),
        "negative_proofs_present": all(item.get("exists") for item in negative_proofs.values()),
        "negative_proofs_rejected": all(item.get("exit_code") != 0 and item.get("result") == "failure" for item in negative_proofs.values()),
        "optimized_full_helper_improved": performance_summary["improved"],
        "optimized_cuda_topology": optimized_topology,
        "fps_debounce_queued_not_modified_here": True,
    }
    checks["ok"] = all(checks.values())
    return {
        "ok": checks["ok"],
        "checks": checks,
        "focused_targets": focused_targets,
        "measurements": measurements,
        "grouped_proofs": grouped_proofs,
        "negative_proofs": negative_proofs,
        "performance_summary": performance_summary,
        "build_script": compile_profile,
        "interpretation": {
            "timeout_cause": "native helper full historically compiled repeated CUDA sources from a clean object directory; optimized topology now reuses common CUDA objects and supports multi-target focused runs from one setup",
            "next_slice": "return to FPS debounce/pacing with measurement artifacts before policy changes",
        },
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Validate native helper build measurement artifacts and focused target coverage")
    parser.add_argument("--out-json", required=True, help="Structured validation report path")
    args = parser.parse_args(argv)

    report = build_report()
    out_path = _repo_path(Path(args.out_json))
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2))
    return 0 if report["ok"] else 2


if __name__ == "__main__":
    raise SystemExit(main())
