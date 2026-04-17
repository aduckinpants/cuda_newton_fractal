from __future__ import annotations

import json
import sys
from pathlib import Path

import pytest


REPO_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO_ROOT / "tools"))

import explaino_runtime_walk as runtime_walk  # noqa: E402
import flashlight_bridge_runner as flashlight_bridge  # noqa: E402


def _write_minimal_fits(path: Path) -> None:
    fits = pytest.importorskip("astropy.io.fits")
    import numpy as np

    primary = fits.PrimaryHDU(data=np.zeros((8, 8, 3), dtype=np.float32))
    op_state = fits.BinTableHDU.from_columns(
        [fits.Column(name="OP_STATE", format="4B", array=np.array([[116, 101, 115, 116]], dtype=np.uint8))],
        name="OP_STATE",
    )
    fits.HDUList([primary, op_state]).writeto(path, overwrite=True)


def test_discover_checkpoint_fits_prefers_explicit_paths_and_ignores_layout_conventions(tmp_path: Path) -> None:
    explicit_dir = tmp_path / "explicit"
    scanned_dir = tmp_path / "nested" / "archive" / "candidate" / "alpha" / "beta" / "gamma"
    explicit_dir.mkdir(parents=True)
    scanned_dir.mkdir(parents=True)

    explicit_fits = explicit_dir / "checkpoint_final.fits"
    scanned_fits = scanned_dir / "checkpoint_final.fits"
    _write_minimal_fits(explicit_fits)
    _write_minimal_fits(scanned_fits)

    discovered = runtime_walk.discover_checkpoint_fits(
        explicit_paths=[explicit_fits],
        search_roots=[tmp_path],
    )

    assert discovered[0] == explicit_fits
    assert scanned_fits in discovered


def test_build_trace_artifacts_from_synthetic_runtime_walk_report(tmp_path: Path) -> None:
    diagnostics_dir = tmp_path
    width = 48
    height = 32
    pixels = [0xFF102030 for _ in range(width * height)]
    reference_frame = diagnostics_dir / "reference_frame.bmp"
    flashlight_bridge._write_bmp32(reference_frame, width, height, pixels)

    report = {
        "reference_view": {
            "frame_bmp": reference_frame.name,
            "lens_sdf_bmp": "reference_lens_sdf.bmp",
        },
        "trace": [
            {
                "reference_trace": {
                    "render_xy": [6, 7],
                    "saddle": {"min_abs_signed_px": 0.5},
                }
            },
            {
                "reference_trace": {
                    "render_xy": [20, 16],
                    "saddle": {"min_abs_signed_px": 0.25},
                }
            },
            {
                "reference_trace": {
                    "render_xy": [40, 26],
                    "saddle": {"min_abs_signed_px": 1.0},
                }
            },
        ],
    }
    report_path = diagnostics_dir / "runtime_walk_report.json"
    report_path.write_text(json.dumps(report), encoding="utf-8")

    artifacts = runtime_walk.build_trace_artifacts(report_path, diagnostics_dir)
    assert Path(artifacts["trace_frame_bmp"]).exists()
    assert Path(artifacts["trace_overlay_bmp"]).exists()
    assert Path(artifacts["trace_stl"]).exists()
    assert Path(artifacts["trace_obj"]).exists()
    assert Path(artifacts["trace_csv"]).exists()


def test_inspect_checkpoint_fits_reports_primary_and_op_state(tmp_path: Path) -> None:
    fits_path = tmp_path / "checkpoint_final.fits"
    _write_minimal_fits(fits_path)

    info = runtime_walk.inspect_checkpoint_fits(fits_path)
    assert info["path"] == str(fits_path)
    assert info["hdu_names"] == ["PRIMARY", "OP_STATE"]
    assert info["primary_shape"] == [8, 8, 3]
    assert info["has_op_state"] is True
