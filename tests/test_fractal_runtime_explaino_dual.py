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


def test_dual_seed_cli_overrides_survive_headless_capture() -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino-DualSeed runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    result = subprocess.run(
        [
            str(exe_path),
            "--capture-diagnostic",
            "--fractal-type",
            "explaino_dual",
            "--explaino-seed",
            "2.25",
            "--explaino-seed-b",
            "7.75",
            "--explaino-mix",
            "0.35",
        ],
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout
    assert DIAGNOSTICS_STATE_FILE.exists(), f"missing diagnostics state file: {DIAGNOSTICS_STATE_FILE}"

    state = json.loads(DIAGNOSTICS_STATE_FILE.read_text(encoding="utf-8"))
    assert state["fractal_type"] == "explaino_dual"
    assert state["params"]["explaino_seed"] == 2
    assert state["view"]["explaino_seed_drift"] == pytest.approx(0.25, abs=1e-6)
    assert state["params"]["explaino_seed_b"] == pytest.approx(7.75, abs=1e-6)
    assert state["params"]["explaino_mix"] == pytest.approx(0.35, abs=1e-6)