from __future__ import annotations

from dataclasses import dataclass
import hashlib
import json
import subprocess
from pathlib import Path

import pytest


RUNTIME_DIR = Path(r"D:\salt-fractal\cuda_newton_fractal_clone\runtime")
ACTIVE_RUNTIME_FILE = RUNTIME_DIR / "fractal_ui_active.txt"
DIAGNOSTICS_STATE_FILE = RUNTIME_DIR / "diagnostics" / "last" / "state.json"
DIAGNOSTICS_FRAME_FILE = RUNTIME_DIR / "diagnostics" / "last" / "frame.bmp"


@dataclass(frozen=True)
class HeadlessLoadedStateScenario:
    name: str
    state: dict[str, object]
    action_args: tuple[str, ...]


@dataclass(frozen=True)
class HeadlessLoadedStateScenarioResult:
    state_path: Path
    baseline_capture: dict[str, object]
    scenario_capture: dict[str, object]


def active_runtime_exe() -> Path:
    if not ACTIVE_RUNTIME_FILE.exists():
        pytest.skip(f"missing active runtime metadata: {ACTIVE_RUNTIME_FILE}")
    active_name = ACTIVE_RUNTIME_FILE.read_text(encoding="utf-8").strip()
    if not active_name:
        pytest.skip(f"empty active runtime metadata: {ACTIVE_RUNTIME_FILE}")
    exe_path = RUNTIME_DIR / active_name
    if not exe_path.exists():
        pytest.skip(f"active runtime missing: {exe_path}")
    return exe_path


def run_headless_capture(*args: str) -> dict[str, object]:
    DIAGNOSTICS_STATE_FILE.unlink(missing_ok=True)
    DIAGNOSTICS_FRAME_FILE.unlink(missing_ok=True)

    result = subprocess.run(
        list(args),
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout
    assert DIAGNOSTICS_STATE_FILE.exists(), f"missing diagnostics state file: {DIAGNOSTICS_STATE_FILE}"
    assert DIAGNOSTICS_FRAME_FILE.exists(), f"missing diagnostics frame file: {DIAGNOSTICS_FRAME_FILE}"
    frame_bytes = DIAGNOSTICS_FRAME_FILE.read_bytes()

    return {
        "state": json.loads(DIAGNOSTICS_STATE_FILE.read_text(encoding="utf-8")),
        "frame_hash": hashlib.sha256(frame_bytes).hexdigest(),
        "frame_bytes": frame_bytes,
    }


def capture_explaino_runtime_baseline(exe_path: Path, *, width: int = 320, height: int = 240) -> dict[str, object]:
    return run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        str(width),
        "--height",
        str(height),
    )


def write_state_bundle(tmp_path: Path, state: dict[str, object]) -> Path:
    state_path = tmp_path / "state.json"
    state_path.parent.mkdir(parents=True, exist_ok=True)
    state_path.write_text(json.dumps(state, indent=2), encoding="utf-8")
    return state_path


def run_headless_loaded_state_scenario(
    tmp_path: Path,
    *,
    exe_path: Path,
    scenario: HeadlessLoadedStateScenario,
) -> HeadlessLoadedStateScenarioResult:
    state_path = write_state_bundle(tmp_path / scenario.name, scenario.state)
    baseline_capture = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--capture-diagnostic",
    )
    scenario_capture = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        *scenario.action_args,
        "--capture-diagnostic",
    )
    return HeadlessLoadedStateScenarioResult(
        state_path=state_path,
        baseline_capture=baseline_capture,
        scenario_capture=scenario_capture,
    )