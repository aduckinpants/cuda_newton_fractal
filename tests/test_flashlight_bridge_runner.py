from __future__ import annotations

import json
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO_ROOT / "tools"))

import flashlight_bridge_runner as bridge  # noqa: E402


def test_build_trace_artifacts_from_synthetic_probe(tmp_path: Path) -> None:
    diagnostics_dir = tmp_path
    width = 64
    height = 48
    pixels = [0xFF101820 for _ in range(width * height)]
    reference_frame = diagnostics_dir / "flashlight_reference_frame.bmp"
    bridge._write_bmp32(reference_frame, width, height, pixels)

    probe = {
        "reference_view": {
            "frame_bmp": reference_frame.name,
            "lens_sdf_bmp": "flashlight_reference_lens_sdf.bmp",
        },
        "trace": [
            {
                "reference_trace": {
                    "render_xy": [8, 10],
                    "saddle": {"min_abs_signed_px": 0.25},
                }
            },
            {
                "reference_trace": {
                    "render_xy": [28, 20],
                    "saddle": {"min_abs_signed_px": 1.0},
                }
            },
            {
                "reference_trace": {
                    "render_xy": [54, 34],
                    "saddle": {"min_abs_signed_px": 0.5},
                }
            },
        ],
    }
    probe_path = diagnostics_dir / "flashlight_probe.json"
    probe_path.write_text(json.dumps(probe), encoding="utf-8")

    artifacts = bridge._build_trace_artifacts(probe_path, diagnostics_dir)
    for key in ("trace_frame_bmp", "trace_overlay_bmp", "trace_stl", "trace_obj", "trace_csv"):
        assert Path(artifacts[key]).exists(), f"missing artifact for {key}"
    stl_bytes = Path(artifacts["trace_stl"]).read_bytes()
    assert stl_bytes[:16].rstrip(b"\0") == b"flashlight_trace"
    tri_count = int.from_bytes(stl_bytes[80:84], "little")
    assert tri_count > 0
