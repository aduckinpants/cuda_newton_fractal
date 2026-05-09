from __future__ import annotations

import json
import subprocess
import sys
import time
from pathlib import Path

import pytest

from tests.runtime_harness import RUNTIME_DIR, active_runtime_exe as _active_runtime_exe


DIAGNOSTICS_LAST_DIR = RUNTIME_DIR / "diagnostics" / "last"


def _mtime_or_zero(path: Path) -> float:
    return path.stat().st_mtime if path.exists() else 0.0


def test_flashlight_probe_regenerates_probe_bundle(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("flashlight probe runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    seed_path = tmp_path / "flashlight_seed.txt"
    seed_path.write_text(
        "orientation drift probe\n"
        "Q1: establish baseline.\n"
        "Q2: force a coherence-preserving correction.\n",
        encoding="utf-8",
    )

    probe_json = DIAGNOSTICS_LAST_DIR / "flashlight_probe.json"
    frame_bmp = DIAGNOSTICS_LAST_DIR / "frame.bmp"
    reference_frame_bmp = DIAGNOSTICS_LAST_DIR / "flashlight_reference_frame.bmp"
    reference_lens_sdf_bmp = DIAGNOSTICS_LAST_DIR / "flashlight_reference_lens_sdf.bmp"
    tick0_bmp = DIAGNOSTICS_LAST_DIR / "frame_000.bmp"
    lens_sdf_bmp = DIAGNOSTICS_LAST_DIR / "lens_sdf.bmp"
    state_json = DIAGNOSTICS_LAST_DIR / "state.json"
    before = {
        probe_json: _mtime_or_zero(probe_json),
        frame_bmp: _mtime_or_zero(frame_bmp),
        reference_frame_bmp: _mtime_or_zero(reference_frame_bmp),
        reference_lens_sdf_bmp: _mtime_or_zero(reference_lens_sdf_bmp),
        tick0_bmp: _mtime_or_zero(tick0_bmp),
        lens_sdf_bmp: _mtime_or_zero(lens_sdf_bmp),
        state_json: _mtime_or_zero(state_json),
    }

    time.sleep(1.1)
    result = subprocess.run(
        [
            str(exe_path),
            "--flashlight-probe",
            str(seed_path),
            "--flashlight-ticks",
            "4",
            "--flashlight-radius",
            "0.61",
            "--flashlight-zoom-radius",
            "0.19",
            "--flashlight-warp",
            "0.2",
        ],
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout

    for path, previous_mtime in before.items():
        assert path.exists(), f"missing flashlight artifact: {path}"
        assert path.stat().st_mtime > previous_mtime, f"artifact was not regenerated: {path}"

    probe = json.loads(probe_json.read_text(encoding="utf-8"))
    assert probe["ticks"] == 4
    assert probe["radius"] == pytest.approx(0.61)
    assert probe["zoom_radius"] == pytest.approx(0.19)
    assert probe["fractal_type"] == "explaino_fp"
    assert probe["conversation_seed32"] > 0
    assert len(probe["spectrum8_u32"]) == 8
    assert len(probe["trace"]) == 4
    assert probe["reference_view"]["frame_bmp"] == "flashlight_reference_frame.bmp"
    assert probe["trace"][0]["reference_trace"]["render_xy"]
    assert probe["summary"]["closure"]["enabled"] is False


def test_flashlight_probe_conflicts_with_describe_functions(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("flashlight probe runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    seed_path = tmp_path / "flashlight_seed.txt"
    seed_path.write_text("conflict check", encoding="utf-8")

    result = subprocess.run(
        [
            str(exe_path),
            "--describe-functions",
            "--flashlight-probe",
            str(seed_path),
        ],
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode != 0
    assert "mutually exclusive" in (result.stderr or result.stdout)
