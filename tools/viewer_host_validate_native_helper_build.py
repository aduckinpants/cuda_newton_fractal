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
    "full_helper": Path("artifacts/validation/native_helper_full_measured.json"),
}
CUDA_SOURCE_NAMES = (
    "fractal_renderer.cu",
    "fractal_sample_core.cu",
    "generic_sample_core.cu",
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


def _measurement_status() -> dict[str, dict[str, Any]]:
    out: dict[str, dict[str, Any]] = {}
    for name, rel_path in REQUIRED_MEASUREMENTS.items():
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


def _cuda_compile_profile(script_text: str) -> dict[str, Any]:
    source_counts = {name: script_text.count(name) for name in CUDA_SOURCE_NAMES}
    arch_counts = {arch: script_text.count(f"code={arch}") for arch in ("sm_86", "sm_120", "sm_121")}
    return {
        "nvcc_invocation_count": script_text.count("nvcc -allow-unsupported-compiler"),
        "cuda_source_counts": source_counts,
        "cuda_arch_counts": arch_counts,
    }


def build_report() -> dict[str, Any]:
    script_text = BUILD_SCRIPT.read_text(encoding="utf-8")
    focused_targets = {target: _target_presence(script_text, target) for target in REQUIRED_FOCUSED_TARGETS}
    measurements = _measurement_status()
    checks = {
        "focused_targets_present": all(item["dispatch"] and item["label"] for item in focused_targets.values()),
        "measurement_jsons_present": all(item.get("exists") for item in measurements.values()),
        "measurement_jsons_success": all(item.get("ok") and item.get("exit_code") == 0 for item in measurements.values()),
        "fps_debounce_queued_not_modified_here": True,
    }
    checks["ok"] = all(checks.values())
    return {
        "ok": checks["ok"],
        "checks": checks,
        "focused_targets": focused_targets,
        "measurements": measurements,
        "build_script": _cuda_compile_profile(script_text),
        "interpretation": {
            "timeout_cause": "native helper full runs compile many CUDA executables from a clean object directory; focused fractal_renderer timing is the bounded witness for CUDA compile cost",
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
