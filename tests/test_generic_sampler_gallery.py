from __future__ import annotations

import json
from pathlib import Path

import pytest

from tools.reality_toolkit.fractal_explorer import generic_sampler_gallery as gallery_mod


def _png_signature(path: Path) -> bytes:
    return path.read_bytes()[:8]


def test_write_generic_sample_gallery_writes_single_grid_frame(tmp_path: Path) -> None:
    response = {
        "function_id": "generic.sample",
        "request_id": "generic-grid-one",
        "summary": {"sample_count": 4},
        "samples": [
            {"sequence_index": 0, "grid_x": 0, "grid_y": 0, "status": "bounded", "value_x": 1.0, "value_y": 0.0, "abs2": 1.0},
            {"sequence_index": 0, "grid_x": 1, "grid_y": 0, "status": "bounded", "value_x": 0.0, "value_y": 1.0, "abs2": 1.0},
            {"sequence_index": 0, "grid_x": 0, "grid_y": 1, "status": "bounded", "value_x": -1.0, "value_y": 0.0, "abs2": 1.0},
            {"sequence_index": 0, "grid_x": 1, "grid_y": 1, "status": "bounded", "value_x": 0.0, "value_y": -1.0, "abs2": 1.0},
        ],
    }

    manifest = gallery_mod.write_generic_sample_gallery(response, tmp_path)

    frame_path = tmp_path / "frame_0000.png"
    manifest_path = tmp_path / "gallery_manifest.json"
    response_path = tmp_path / "response.json"

    assert frame_path.exists()
    assert manifest_path.exists()
    assert response_path.exists()
    assert _png_signature(frame_path) == b"\x89PNG\r\n\x1a\n"
    assert manifest["frame_count"] == 1
    assert manifest["frames"][0]["sequence_index"] == 0
    assert manifest["frames"][0]["width"] == 2
    assert manifest["frames"][0]["height"] == 2


def test_write_generic_sample_gallery_writes_sequence_grid_frames(tmp_path: Path) -> None:
    response = {
        "function_id": "generic.sample",
        "request_id": "generic-sequence-grid",
        "summary": {"sample_count": 8},
        "sequence_results": [
            {"sequence_index": 0, "applied": {"function.params.scale_real": 0.5}},
            {"sequence_index": 1, "applied": {"function.params.scale_real": 1.0}},
        ],
        "samples": [
            {"sequence_index": 0, "grid_x": 0, "grid_y": 0, "status": "bounded", "value_x": 1.0, "value_y": 0.0, "abs2": 1.0},
            {"sequence_index": 0, "grid_x": 1, "grid_y": 0, "status": "bounded", "value_x": 0.0, "value_y": 1.0, "abs2": 1.0},
            {"sequence_index": 0, "grid_x": 0, "grid_y": 1, "status": "bounded", "value_x": -1.0, "value_y": 0.0, "abs2": 1.0},
            {"sequence_index": 0, "grid_x": 1, "grid_y": 1, "status": "bounded", "value_x": 0.0, "value_y": -1.0, "abs2": 1.0},
            {"sequence_index": 1, "grid_x": 0, "grid_y": 0, "status": "bounded", "value_x": 2.0, "value_y": 0.0, "abs2": 4.0},
            {"sequence_index": 1, "grid_x": 1, "grid_y": 0, "status": "bounded", "value_x": 0.0, "value_y": 2.0, "abs2": 4.0},
            {"sequence_index": 1, "grid_x": 0, "grid_y": 1, "status": "bounded", "value_x": -2.0, "value_y": 0.0, "abs2": 4.0},
            {"sequence_index": 1, "grid_x": 1, "grid_y": 1, "status": "bounded", "value_x": 0.0, "value_y": -2.0, "abs2": 4.0},
        ],
    }

    manifest = gallery_mod.write_generic_sample_gallery(response, tmp_path)

    frame0 = tmp_path / "frame_0000.png"
    frame1 = tmp_path / "frame_0001.png"
    assert frame0.exists()
    assert frame1.exists()
    assert frame0.read_bytes() != frame1.read_bytes()

    manifest_json = json.loads((tmp_path / "gallery_manifest.json").read_text(encoding="utf-8"))
    assert manifest == manifest_json
    assert manifest["frame_count"] == 2
    assert manifest["frames"][0]["applied"]["function.params.scale_real"] == pytest.approx(0.5)
    assert manifest["frames"][1]["applied"]["function.params.scale_real"] == pytest.approx(1.0)


def test_write_generic_sample_gallery_rejects_missing_grid_coordinates(tmp_path: Path) -> None:
    response = {
        "function_id": "generic.sample",
        "request_id": "generic-bad-grid",
        "summary": {"sample_count": 1},
        "samples": [
            {"sequence_index": 0, "grid_x": -1, "grid_y": 0, "status": "bounded", "value_x": 1.0, "value_y": 0.0, "abs2": 1.0},
        ],
    }

    with pytest.raises(ValueError, match="grid"):
        gallery_mod.write_generic_sample_gallery(response, tmp_path)


def test_write_generic_sample_gallery_uses_iterations_to_break_root_color_ties(tmp_path: Path) -> None:
    low_iter_response = {
        "function_id": "generic.sample",
        "request_id": "generic-iter-low",
        "summary": {"sample_count": 2},
        "samples": [
            {"sequence_index": 0, "grid_x": 0, "grid_y": 0, "iterations": 4, "status": "converged", "value_x": 1.0, "value_y": 0.0, "abs2": 1.0},
            {"sequence_index": 0, "grid_x": 1, "grid_y": 0, "iterations": 4, "status": "converged", "value_x": 1.0, "value_y": 0.0, "abs2": 1.0},
        ],
    }
    high_iter_response = {
        "function_id": "generic.sample",
        "request_id": "generic-iter-high",
        "summary": {"sample_count": 2},
        "samples": [
            {"sequence_index": 0, "grid_x": 0, "grid_y": 0, "iterations": 12, "status": "converged", "value_x": 1.0, "value_y": 0.0, "abs2": 1.0},
            {"sequence_index": 0, "grid_x": 1, "grid_y": 0, "iterations": 12, "status": "converged", "value_x": 1.0, "value_y": 0.0, "abs2": 1.0},
        ],
    }

    gallery_mod.write_generic_sample_gallery(low_iter_response, tmp_path / "low")
    gallery_mod.write_generic_sample_gallery(high_iter_response, tmp_path / "high")

    low_bytes = (tmp_path / "low" / "frame_0000.png").read_bytes()
    high_bytes = (tmp_path / "high" / "frame_0000.png").read_bytes()
    assert low_bytes != high_bytes