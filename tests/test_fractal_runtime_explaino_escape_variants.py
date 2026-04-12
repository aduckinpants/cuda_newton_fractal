from __future__ import annotations

import hashlib
import json
import subprocess
import sys
from pathlib import Path

import pytest


RUNTIME_DIR = Path(r"D:\salt-fractal\cuda_newton_fractal_clone\runtime")
ACTIVE_RUNTIME_FILE = RUNTIME_DIR / "fractal_ui_active.txt"
DIAGNOSTICS_STATE_FILE = RUNTIME_DIR / "diagnostics" / "last" / "state.json"
DIAGNOSTICS_FRAME_FILE = RUNTIME_DIR / "diagnostics" / "last" / "frame.bmp"


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


def _run_headless_capture(*args: str) -> dict[str, object]:
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

    return {
        "state": json.loads(DIAGNOSTICS_STATE_FILE.read_text(encoding="utf-8")),
        "frame_hash": hashlib.sha256(DIAGNOSTICS_FRAME_FILE.read_bytes()).hexdigest(),
    }


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


def test_explaino_composed_variants_render_distinct_default_frames() -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino composed runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    hashes: dict[str, str] = {}
    expected_strengths = {
        "explaino_ripple": {
            "ripple_amplitude": 0.15,
            "splice_offset": 0.0,
            "vortex_strength": 0.0,
            "tension_strength": 0.0,
        },
        "explaino_splice": {
            "ripple_amplitude": 0.0,
            "splice_offset": 0.5,
            "vortex_strength": 0.0,
            "tension_strength": 0.0,
        },
        "explaino_vortex": {
            "ripple_amplitude": 0.0,
            "splice_offset": 0.0,
            "vortex_strength": 0.3,
            "tension_strength": 0.0,
        },
        "explaino_tension": {
            "ripple_amplitude": 0.0,
            "splice_offset": 0.0,
            "vortex_strength": 0.0,
            "tension_strength": 0.02,
        },
    }
    for variant, expected_params in expected_strengths.items():
        capture = _run_headless_capture(
            str(exe_path),
            "--capture-diagnostic",
            "--fractal-type",
            variant,
            "--width",
            "320",
            "--height",
            "240",
        )
        state = capture["state"]
        assert state["fractal_type"] == variant
        for param_name, expected_value in expected_params.items():
            assert state["params"][param_name] == pytest.approx(expected_value, abs=1e-6)
        hashes[variant] = str(capture["frame_hash"])

    assert len(set(hashes.values())) == len(hashes), f"expected distinct default frame hashes, got {hashes}"


def test_explaino_composed_variant_state_round_trips_through_load_state_json(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino composed state round-trip regression is Windows-only")

    exe_path = _active_runtime_exe()
    initial_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino_vortex",
        "--width",
        "320",
        "--height",
        "240",
    )
    saved_state_path = tmp_path / "state.json"
    saved_state_path.write_bytes(DIAGNOSTICS_STATE_FILE.read_bytes())

    reloaded_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(saved_state_path),
        "--capture-diagnostic",
    )

    initial_state = initial_capture["state"]
    reloaded_state = reloaded_capture["state"]
    assert reloaded_state["fractal_type"] == "explaino_vortex"
    assert reloaded_state["params"]["vortex_strength"] == pytest.approx(0.3, abs=1e-6)
    assert reloaded_state["params"]["ripple_amplitude"] == pytest.approx(0.0, abs=1e-6)
    assert "sidecar_orientation" in initial_state
    assert reloaded_state["sidecar_orientation"] == initial_state["sidecar_orientation"]
    assert "sidecar_auto_demo_policy" in initial_state
    assert reloaded_state["sidecar_auto_demo_policy"] == initial_state["sidecar_auto_demo_policy"]
    assert reloaded_capture["frame_hash"] == initial_capture["frame_hash"]
    assert reloaded_state["render"]["width"] == initial_state["render"]["width"]
    assert reloaded_state["render"]["height"] == initial_state["render"]["height"]