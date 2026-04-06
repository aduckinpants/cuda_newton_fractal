from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

import pytest


RUNTIME_DIR = Path(r"D:\salt-fractal\cuda_newton_fractal_clone\runtime")
ACTIVE_RUNTIME_FILE = RUNTIME_DIR / "fractal_ui_active.txt"
DIAGNOSTICS_STATE_FILE = RUNTIME_DIR / "diagnostics" / "last" / "state.json"


def _active_runtime_exe() -> Path:
    if not ACTIVE_RUNTIME_FILE.exists():
        pytest.skip(f"missing active runtime metadata: {ACTIVE_RUNTIME_FILE}")
    active_name = ACTIVE_RUNTIME_FILE.read_text(encoding="utf-8").strip()
    if not active_name:
        pytest.skip(f"empty active runtime metadata: {ACTIVE_RUNTIME_FILE}")
    exe_path = RUNTIME_DIR / active_name
    if not exe_path.exists():
        pytest.skip(f"active runtime missing: {exe_path}")
    return exe_path


def test_explaino_lambda_cli_overrides_survive_headless_capture() -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino-Lambda runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    result = subprocess.run(
        [
            str(exe_path),
            "--capture-diagnostic",
            "--fractal-type",
            "explaino_lambda",
            "--lambda-real",
            "2.9685855",
            "--lambda-imag",
            "-0.27446103",
            "--explaino-seed",
            "3.25",
            "--explaino-warp-strength",
            "0.2",
        ],
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout
    assert DIAGNOSTICS_STATE_FILE.exists(), f"missing diagnostics state file: {DIAGNOSTICS_STATE_FILE}"

    state = json.loads(DIAGNOSTICS_STATE_FILE.read_text(encoding="utf-8"))
    assert state["fractal_type"] == "explaino_lambda"
    assert state["params"]["lambda_real"] == pytest.approx(2.9685855, abs=1e-5)
    assert state["params"]["lambda_imag"] == pytest.approx(-0.27446103, abs=1e-5)
    assert state["params"]["explaino_seed"] == 3
    assert state["view"]["explaino_seed_drift"] == pytest.approx(0.25, abs=1e-6)
    assert state["params"]["explaino_warp_strength"] == pytest.approx(0.2, abs=1e-6)
    assert state["params"]["coloring_mode"] == "smooth_escape"


def test_explaino_rational_escape_cli_overrides_survive_headless_capture() -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino-Rational-Escape runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    result = subprocess.run(
        [
            str(exe_path),
            "--capture-diagnostic",
            "--fractal-type",
            "explaino_rational_escape",
            "--explaino-seed",
            "4.2",
            "--explaino-warp-strength",
            "0.35",
            "--width",
            "640",
            "--height",
            "480",
        ],
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout
    assert DIAGNOSTICS_STATE_FILE.exists(), f"missing diagnostics state file: {DIAGNOSTICS_STATE_FILE}"

    state = json.loads(DIAGNOSTICS_STATE_FILE.read_text(encoding="utf-8"))
    assert state["fractal_type"] == "explaino_rational_escape"
    assert state["params"]["explaino_seed"] == 4
    assert state["view"]["explaino_seed_drift"] == pytest.approx(0.2, abs=1e-6)
    assert state["params"]["explaino_warp_strength"] == pytest.approx(0.35, abs=1e-6)
    assert state["params"]["coloring_mode"] == "smooth_escape"
    assert state["render"]["width"] == 640
    assert state["render"]["height"] == 480