from __future__ import annotations

from io import BytesIO
from pathlib import Path
import subprocess

import pytest

from tests.runtime_harness import (
    DIAGNOSTICS_FRAME_FILE,
    DIAGNOSTICS_STATE_FILE,
    RUNTIME_DIR,
    active_runtime_exe as _active_runtime_exe,
)


MANUAL_CAPTURE_DIR = Path(
    r"D:\salt-fractal\cuda_newton_fractal_clone\findings\manual_capture\2026-05-11\234919_563__explaino_inertial"
)
MANUAL_CAPTURE_STATE = MANUAL_CAPTURE_DIR / "state.json"
MANUAL_CAPTURE_FRAME = MANUAL_CAPTURE_DIR / "frame.png"


def _image_mean_abs_rgb(left, right) -> float:
    left_pixels = list(left.getdata())
    right_pixels = list(right.getdata())
    assert len(left_pixels) == len(right_pixels)
    total = 0
    for left_rgb, right_rgb in zip(left_pixels, right_pixels):
        total += abs(left_rgb[0] - right_rgb[0])
        total += abs(left_rgb[1] - right_rgb[1])
        total += abs(left_rgb[2] - right_rgb[2])
    return total / float(len(left_pixels) * 3)


def _resampling_lanczos():
    image_module = pytest.importorskip("PIL.Image")
    return getattr(getattr(image_module, "Resampling", image_module), "LANCZOS")


def test_manual_explaino_inertial_capture_reloads_to_detailed_frame() -> None:
    image_module = pytest.importorskip("PIL.Image")
    if not MANUAL_CAPTURE_STATE.exists() or not MANUAL_CAPTURE_FRAME.exists():
        pytest.skip(f"missing manual Explaino-Inertial capture fixture: {MANUAL_CAPTURE_DIR}")

    exe_path = _active_runtime_exe()
    DIAGNOSTICS_STATE_FILE.unlink(missing_ok=True)
    DIAGNOSTICS_FRAME_FILE.unlink(missing_ok=True)

    result = subprocess.run(
        [
            str(exe_path),
            "--load-state-json",
            str(MANUAL_CAPTURE_STATE),
            "--width",
            "256",
            "--height",
            "256",
            "--capture-diagnostic",
        ],
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout
    assert DIAGNOSTICS_STATE_FILE.exists(), f"missing diagnostics state file: {DIAGNOSTICS_STATE_FILE}"
    assert DIAGNOSTICS_FRAME_FILE.exists(), f"missing diagnostics frame file: {DIAGNOSTICS_FRAME_FILE}"

    loaded_state = __import__("json").loads(DIAGNOSTICS_STATE_FILE.read_text(encoding="utf-8"))
    actual = image_module.open(BytesIO(DIAGNOSTICS_FRAME_FILE.read_bytes())).convert("RGB")
    reference = image_module.open(MANUAL_CAPTURE_FRAME).convert("RGB").resize(actual.size, _resampling_lanczos())
    actual_unique_colors = len(set(actual.getdata()))
    mean_abs_rgb = _image_mean_abs_rgb(reference, actual)

    failures = []
    if loaded_state["params"]["multibrot_power"] != 3:
        failures.append(f"multibrot_power reloaded as {loaded_state['params']['multibrot_power']}, expected 3")
    if actual_unique_colors <= 1000:
        failures.append(f"reloaded frame has only {actual_unique_colors} unique colors")
    if mean_abs_rgb >= 35.0:
        failures.append(f"reloaded frame mean_abs_rgb={mean_abs_rgb:.3f}, expected < 35.0")
    assert not failures, "\n".join(failures)
