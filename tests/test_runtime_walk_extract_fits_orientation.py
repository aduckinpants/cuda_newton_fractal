from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

import pytest


REPO_ROOT = Path(__file__).resolve().parents[1]


def test_runtime_walk_extract_fits_orientation_writes_expected_signals(tmp_path: Path) -> None:
    fits = pytest.importorskip("astropy.io.fits")
    np = pytest.importorskip("numpy")

    fits_path = tmp_path / "checkpoint_final.fits"
    out_json = tmp_path / "orientation_inputs.json"
    data = np.array(
        [
            [0.0, 0.2, 0.4, 0.6],
            [0.1, 0.3, 0.5, 0.7],
            [0.2, 0.4, 0.6, 0.8],
            [0.3, 0.5, 0.7, 0.9],
        ],
        dtype=np.float32,
    )
    fits.PrimaryHDU(data).writeto(fits_path)

    script = REPO_ROOT / "tools" / "runtime_walk_extract_fits_orientation.py"
    subprocess.run(
        [
            sys.executable,
            str(script),
            "--fits",
            str(fits_path),
            "--out-json",
            str(out_json),
        ],
        cwd=str(REPO_ROOT),
        check=True,
        capture_output=True,
        text=True,
    )

    payload = json.loads(out_json.read_text(encoding="utf-8"))
    assert payload["version"] == 1
    assert Path(payload["fits_path"]) == fits_path.resolve()
    signals = payload["signals"]
    for key in (
        "mean",
        "stddev",
        "residual_energy",
        "frame_delta",
        "focus_ratio",
        "center_bias",
        "edge_balance",
        "x_bias",
        "y_bias",
    ):
        assert key in signals
        assert isinstance(signals[key], (float, int))
