from __future__ import annotations

import json
import subprocess
import sys
import time
from pathlib import Path

import pytest


REPO_ROOT = Path(__file__).resolve().parents[1]
RUNTIME_DIR = Path(r"D:\salt-fractal\cuda_newton_fractal_clone\runtime")
DIAGNOSTICS_LAST_DIR = RUNTIME_DIR / "diagnostics" / "last"


def _mtime_or_zero(path: Path) -> float:
    return path.stat().st_mtime if path.exists() else 0.0


def test_flashlight_bridge_runner_emits_trace_artifacts(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("flashlight bridge runtime regression is Windows-only")

    seed_path = tmp_path / "flashlight_seed.txt"
    seed_path.write_text(
        "control prompt set\n"
        "Q1 baseline\n"
        "Q2 challenge the orientation\n"
        "Q3 nudge back toward coherence\n",
        encoding="utf-8",
    )
    request_path = DIAGNOSTICS_LAST_DIR / "flashlight_bridge_request.json"
    request = {
        "version": 1,
        "kind": "flashlight_probe_headless",
        "request_id": int(time.time()),
        "seed_path": str(seed_path),
        "ticks": 5,
        "warp": 0.2,
        "closure_last": False,
        "no_export": False,
        "radius": 0.61,
        "zoom_radius": 0.19,
    }
    request_path.write_text(json.dumps(request, indent=2) + "\n", encoding="utf-8")

    expected = {
        DIAGNOSTICS_LAST_DIR / "flashlight_probe.json": _mtime_or_zero(DIAGNOSTICS_LAST_DIR / "flashlight_probe.json"),
        DIAGNOSTICS_LAST_DIR / "flashlight_trace_frame.bmp": _mtime_or_zero(DIAGNOSTICS_LAST_DIR / "flashlight_trace_frame.bmp"),
        DIAGNOSTICS_LAST_DIR / "flashlight_trace_overlay.bmp": _mtime_or_zero(DIAGNOSTICS_LAST_DIR / "flashlight_trace_overlay.bmp"),
        DIAGNOSTICS_LAST_DIR / "flashlight_trace.stl": _mtime_or_zero(DIAGNOSTICS_LAST_DIR / "flashlight_trace.stl"),
        DIAGNOSTICS_LAST_DIR / "flashlight_bridge_status.json": _mtime_or_zero(DIAGNOSTICS_LAST_DIR / "flashlight_bridge_status.json"),
    }

    time.sleep(1.1)
    result = subprocess.run(
        [
            sys.executable,
            str(REPO_ROOT / "tools" / "flashlight_bridge_runner.py"),
            "--runtime-dir",
            str(RUNTIME_DIR),
            "--request-json",
            str(request_path),
        ],
        cwd=str(REPO_ROOT),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout

    for path, previous_mtime in expected.items():
        assert path.exists(), f"missing bridge artifact: {path}"
        assert path.stat().st_mtime > previous_mtime, f"artifact was not regenerated: {path}"

    status = json.loads((DIAGNOSTICS_LAST_DIR / "flashlight_bridge_status.json").read_text(encoding="utf-8"))
    assert status["ok"] is True
    assert Path(status["artifacts"]["trace_stl"]).exists()
    assert Path(status["artifacts"]["trace_overlay_bmp"]).exists()
