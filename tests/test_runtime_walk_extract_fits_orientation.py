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

def test_runtime_walk_extract_fits_orientation_preserves_frame_timeline(tmp_path: Path) -> None:
    fits = pytest.importorskip("astropy.io.fits")
    np = pytest.importorskip("numpy")

    fits_path = tmp_path / "multi_frame_orientation.fits"
    out_json = tmp_path / "orientation_inputs.json"
    data = np.stack(
        [
            np.full((4, 4), 0.10, dtype=np.float32),
            np.eye(4, dtype=np.float32) * 0.75,
            np.tile(np.linspace(0.0, 1.0, 4, dtype=np.float32), (4, 1)) * 1.25,
        ],
        axis=0,
    )
    hdu = fits.PrimaryHDU(data)
    hdu.header["ORIENT"] = "godel-test"
    hdu.writeto(fits_path)

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
    assert payload["frame_count"] == 3
    assert payload["metadata"]["ORIENT"] == "godel-test"
    frames = payload["frames"]
    assert [frame["frame_index"] for frame in frames] == [0, 1, 2]
    assert [frame["t"] for frame in frames] == [0.0, 0.5, 1.0]
    assert frames[0]["signals"]["mean"] != frames[1]["signals"]["mean"]
    assert frames[1]["signals"]["x_bias"] != frames[2]["signals"]["x_bias"]
