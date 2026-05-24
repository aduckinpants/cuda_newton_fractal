from __future__ import annotations

import importlib.util
import json
from pathlib import Path


def _load_module():
    module_path = Path(__file__).resolve().parents[1] / "tools" / "smooth_escape_color_inventory.py"
    spec = importlib.util.spec_from_file_location("smooth_escape_color_inventory", module_path)
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _write_bmp_24(path: Path, width: int, height: int, pixels_top_down: list[tuple[int, int, int]]) -> None:
    row_stride = width * 3
    padding = (4 - (row_stride % 4)) % 4
    pixel_bytes = (row_stride + padding) * height
    file_size = 14 + 40 + pixel_bytes
    data = bytearray()
    data += b"BM"
    data += file_size.to_bytes(4, "little")
    data += (0).to_bytes(2, "little")
    data += (0).to_bytes(2, "little")
    data += (54).to_bytes(4, "little")
    data += (40).to_bytes(4, "little")
    data += width.to_bytes(4, "little", signed=True)
    data += height.to_bytes(4, "little", signed=True)
    data += (1).to_bytes(2, "little")
    data += (24).to_bytes(2, "little")
    data += (0).to_bytes(4, "little")
    data += pixel_bytes.to_bytes(4, "little")
    data += (2835).to_bytes(4, "little", signed=True)
    data += (2835).to_bytes(4, "little", signed=True)
    data += (0).to_bytes(4, "little")
    data += (0).to_bytes(4, "little")
    rows = [
        pixels_top_down[row_index * width : (row_index + 1) * width]
        for row_index in range(height)
    ]
    for row in reversed(rows):
        for red, green, blue in row:
            data += bytes([blue, green, red])
        data += b"\x00" * padding
    path.write_bytes(bytes(data))


def test_frame_metrics_classify_black_fraction_and_color_range(tmp_path: Path) -> None:
    inventory = _load_module()
    bmp_path = tmp_path / "frame.bmp"
    _write_bmp_24(
        bmp_path,
        2,
        2,
        [
            (0, 0, 0),
            (255, 0, 0),
            (0, 128, 0),
            (0, 0, 255),
        ],
    )

    frame = inventory.read_bmp_rgb(bmp_path)
    metrics = inventory.compute_frame_metrics(frame)

    assert metrics["width"] == 2
    assert metrics["height"] == 2
    assert metrics["pixel_count"] == 4
    assert metrics["black_pixel_fraction"] == 0.25
    assert metrics["unique_rgb_count"] == 4
    assert metrics["luma_min"] == 0
    assert metrics["luma_max"] > metrics["luma_min"]


def test_build_case_summary_reads_capture_bundle_state_and_frame(tmp_path: Path) -> None:
    inventory = _load_module()
    bundle = tmp_path / "mandelbrot"
    bundle.mkdir()
    _write_bmp_24(bundle / "frame.bmp", 1, 1, [(16, 32, 64)])
    (bundle / "state.json").write_text(
        json.dumps(
            {
                "fractal_type": "mandelbrot",
                "params": {
                    "coloring_mode": "smooth_escape",
                    "color_signal": "smooth_escape",
                    "color_palette": "cyclic_escape",
                    "color_grading": "escape_default",
                    "color_smooth_escape_scale": 1,
                    "color_smooth_escape_bias": 0,
                    "color_heatmap_cycle_scale": 1,
                },
                "render": {
                    "width": 1,
                    "height": 1,
                    "sample_tier": "tier_auto",
                },
                "stats": {
                    "last_render_ms": 1.25,
                    "last_iters_avg": 12,
                    "resolved_backend": "float32",
                    "resolved_strategy": "direct",
                },
            }
        ),
        encoding="utf-8",
    )

    summary = inventory.build_case_summary("mandelbrot", bundle)

    assert summary["fractal_type"] == "mandelbrot"
    assert summary["reported_fractal_type"] == "mandelbrot"
    assert summary["color"]["mode"] == "smooth_escape"
    assert summary["color"]["signal"] == "smooth_escape"
    assert summary["render"]["sample_tier"] == "tier_auto"
    assert summary["stats"]["resolved_backend"] == "float32"
    assert summary["frame_metrics"]["unique_rgb_count"] == 1


def test_inventory_out_dir_is_absolute_before_runtime_capture(tmp_path: Path, monkeypatch) -> None:
    inventory = _load_module()
    monkeypatch.chdir(tmp_path)

    resolved = inventory.resolve_inventory_out_dir("relative_report")

    assert resolved == tmp_path / "relative_report"
    assert resolved.is_absolute()


def test_inventory_analysis_flags_shared_tuple_and_candidate_cases() -> None:
    inventory = _load_module()
    cases = [
        {
            "fractal_type": "magnet",
            "color": {
                "signal": "smooth_escape",
                "palette": "cyclic_escape",
                "grading": "escape_default",
                "smooth_escape_scale": 1,
                "smooth_escape_bias": 0,
                "heatmap_cycle_scale": 1,
            },
            "frame_metrics": {
                "black_pixel_fraction": 0.75,
                "luma_min": 0,
                "luma_max": 120,
                "unique_rgb_count": 80,
            },
        },
        {
            "fractal_type": "julia",
            "color": {
                "signal": "smooth_escape",
                "palette": "cyclic_escape",
                "grading": "escape_default",
                "smooth_escape_scale": 1,
                "smooth_escape_bias": 0,
                "heatmap_cycle_scale": 1,
            },
            "frame_metrics": {
                "black_pixel_fraction": 0.01,
                "luma_min": 0,
                "luma_max": 180,
                "unique_rgb_count": 500,
            },
        },
    ]

    analysis = inventory.build_inventory_analysis(cases)

    assert analysis["all_cases_share_color_tuple"] is True
    assert analysis["high_black_fraction_cases"] == ["magnet"]
    assert analysis["low_unique_color_cases"] == ["magnet"]
    assert analysis["later_tuning_candidate_cases"] == ["magnet"]
