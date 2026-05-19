from __future__ import annotations

import argparse
import json
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]

REQUIRED_SNIPPETS = {
    "ui_app/src/fractal_types.h": ["magnet = 43", "float magnet_seed_real", "float magnet_relaxation", "float magnet_bailout"],
    "ui_app/src/escape_time_direct_formulas.h": ["FractalType::magnet", "EscapeTimeDirectDiv", "EscapeTimeDirectMagnetResidualSquared", "magnetRelaxation"],
    "ui_app/src/fractal_sample_device.inl": ["params.magnet_seed_real", "params.magnet_relaxation", "params.magnet_bailout", "EscapeTimeDirectMagnetResidualSquared"],
    "ui_app/src/fractal_probe_runner.cpp": ["params.magnet_seed_real", "FractalType::magnet", "EscapeTimeDirectMagnetResidualSquared"],
    "ui_app/src/schema_binding.cpp": ["fractal.params.magnet_seed_real", "fractal.params.magnet_seed_imag", "fractal.params.magnet_relaxation", "fractal.params.magnet_bailout"],
    "ui/fractal_binding_surface_v1.ui_schema.json": ['"id": "magnet"', '"id": "magnet_seed_real"', '"id": "magnet_seed_imag"', '"id": "magnet_relaxation"', '"id": "magnet_bailout"'],
    "ui_app/src/diagnostics_capture.cpp": ["magnet_seed_real", "magnet_relaxation"],
    "ui_app/src/diagnostics_state_io.cpp": ["magnetSeedReal", "magnetBailout"],
    "ui_app/src/function_descriptor.cpp": ["FractalType::magnet"],
    "tests/test_fractal_runtime_magnet.py": ["--ui-automation-set-control-value", "fractal_control.magnet_relaxation.primary", "set_value_consumed"],
    "ui_app/tests/test_fractal_sample_device.cu": ["FractalType::magnet"],
    "ui_app/tests/test_fractal_sample_kernel.cu": ["FractalType::magnet"],
    "ui_app/tests/test_fractal_renderer.cu": ["TestMagnetRenderRespondsToRelaxation"],
    "ui_app/tests/test_diagnostics_capture.cpp": ["TestBundlePersistsMagnetParams"],
}

BANNED_RUNTIME_TEST_TOKENS = ["SetCursorPos", "mouse_event", "pyautogui", "dragTo", "moveTo"]


def _read(rel: str) -> str:
    return (REPO_ROOT / rel).read_text(encoding="utf-8")


def build_report() -> dict[str, object]:
    findings: list[dict[str, object]] = []
    for rel, snippets in REQUIRED_SNIPPETS.items():
        try:
            text = _read(rel)
        except FileNotFoundError:
            findings.append({"severity": "error", "code": "missing_file", "path": rel})
            continue
        for snippet in snippets:
            if snippet not in text:
                findings.append({"severity": "error", "code": "missing_snippet", "path": rel, "snippet": snippet})
    runtime_test = _read("tests/test_fractal_runtime_magnet.py")
    for token in BANNED_RUNTIME_TEST_TOKENS:
        if token in runtime_test:
            findings.append({"severity": "error", "code": "physical_mouse_runtime_test", "path": "tests/test_fractal_runtime_magnet.py", "token": token})
    return {"schema_version": "fractal_toolkit_scan.v1", "ok": not findings, "findings": findings, "checked_files": sorted(REQUIRED_SNIPPETS)}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Validate modular fractal-toolkit architecture rules.")
    parser.add_argument("--out-json", required=True)
    args = parser.parse_args(argv)
    report = build_report()
    out = Path(args.out_json)
    if not out.is_absolute():
        out = REPO_ROOT / out
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(json.dumps({"ok": report["ok"], "out": str(out)}, indent=2))
    return 0 if report["ok"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
