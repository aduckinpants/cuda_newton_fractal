from __future__ import annotations

import importlib.util
import subprocess
import sys
from pathlib import Path

import pytest

from tests.runtime_harness import (
    PersistentRuntimeViewerAutomation,
    active_runtime_exe,
    runtime_automation_lock,
)


def _load_inventory_module():
    module_path = Path(__file__).resolve().parents[1] / "tools" / "smooth_escape_color_inventory.py"
    spec = importlib.util.spec_from_file_location("smooth_escape_color_inventory", module_path)
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def _yellow_fraction(pixels: list[tuple[int, int, int]]) -> float:
    yellow = 0
    for red, green, blue in pixels:
        if red > 150 and green > 130 and blue < 90:
            yellow += 1
    return yellow / len(pixels)


def _capture_default_escape_time_frame(exe_path: Path, out_dir: Path, fractal_type: str) -> dict[str, object]:
    result = subprocess.run(
        [
            str(exe_path),
            "--capture-diagnostic",
            "--out-dir",
            str(out_dir),
            "--fractal-type",
            fractal_type,
            "--width",
            "96",
            "--height",
            "72",
        ],
        cwd=str(Path(r"D:\salt-fractal\cuda_newton_fractal_clone\runtime")),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout
    return {
        "frame_path": out_dir / "frame.bmp",
        "state_path": out_dir / "state.json",
    }


def test_default_escape_time_smooth_escape_interiors_are_not_yellow_dominant(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("published viewer runtime capture is Windows-only")

    inventory = _load_inventory_module()
    exe_path = active_runtime_exe()
    captures = {
        fractal_type: _capture_default_escape_time_frame(exe_path, tmp_path / f"{fractal_type}_default", fractal_type)
        for fractal_type in ("mandelbrot", "multibrot")
    }

    for fractal_type, capture in captures.items():
        frame = inventory.read_bmp_rgb(capture["frame_path"])
        metrics = inventory.compute_frame_metrics(frame)
        yellow_fraction = _yellow_fraction(frame["pixels"])

        assert metrics["black_pixel_fraction"] < 0.05, fractal_type
        assert yellow_fraction < 0.45, {
            "fractal_type": fractal_type,
            "yellow_fraction": yellow_fraction,
            "unique_rgb_count": metrics["unique_rgb_count"],
            "luma_min": metrics["luma_min"],
            "luma_max": metrics["luma_max"],
        }


def test_smooth_escape_interior_strength_is_no_mouse_runtime_control(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("published viewer runtime automation is Windows-only")

    exe_path = active_runtime_exe()
    capture = _capture_default_escape_time_frame(exe_path, tmp_path / "multibrot_control_seed", "multibrot")
    state_path = capture["state_path"]
    state_text = state_path.read_text(encoding="utf-8")
    assert '"color_smooth_escape_interior_strength": 0.2' in state_text

    control_id = "fractal_control.color_smooth_escape_interior_strength.primary"
    with runtime_automation_lock(timeout_seconds=20.0):
        with PersistentRuntimeViewerAutomation(
            exe_path=exe_path,
            state_path=state_path,
            report_path=tmp_path / "interior_strength_report.json",
            command_path=tmp_path / "interior_strength_command.json",
        ) as viewer:
            baseline = viewer.wait_for_report(timeout_seconds=20.0)
            baseline_hash = baseline.get("rendered_frame_hash")
            assert baseline.get("current_fractal_type") == "multibrot"
            assert isinstance(baseline_hash, str) and baseline_hash

            viewer.wait_for_control(control_id, timeout_seconds=15.0)
            edited = viewer.set_control_value(control_id, 1.0, timeout_seconds=20.0)
            edited_hash = edited.get("rendered_frame_hash")
            assert edited.get("current_fractal_type") == "multibrot"
            assert edited.get("set_value_consumed") is True
            assert isinstance(edited_hash, str) and edited_hash
            assert edited_hash != baseline_hash
