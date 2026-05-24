from __future__ import annotations

import importlib.util
import re
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


def _frame_delta_metrics(
    left_pixels: list[tuple[int, int, int]],
    right_pixels: list[tuple[int, int, int]],
) -> dict[str, float]:
    assert len(left_pixels) == len(right_pixels)
    total_delta = 0
    changed = 0
    max_delta = 0
    for left, right in zip(left_pixels, right_pixels):
        delta = abs(left[0] - right[0]) + abs(left[1] - right[1]) + abs(left[2] - right[2])
        total_delta += delta
        if delta:
            changed += 1
        max_delta = max(max_delta, delta)
    return {
        "avg_abs_rgb_delta_per_channel": total_delta / (len(left_pixels) * 3),
        "changed_pixel_fraction": changed / len(left_pixels),
        "max_rgb_sum_delta": float(max_delta),
    }


def _write_state_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="\n") as handle:
        handle.write(text)


def _with_source_stack(state_text: str, signal: str) -> str:
    if '"color_source_stack"' in state_text:
        return state_text
    if signal == "smooth_escape":
        row = '{ "signal": "smooth_escape", "scale": 1.0, "bias": 0.0, "blend_weight": 1.0 }'
    elif signal == "escape_magnitude":
        row = '{ "signal": "escape_magnitude", "magnitude_scale": 1.5, "magnitude_bias": -0.25, "blend_weight": 1.0 }'
    else:
        raise ValueError(f"unsupported source stack signal: {signal}")
    state_text = re.sub(r'("color_signal"\s*:\s*)"[^"]+"', rf'\g<1>"{signal}"', state_text, count=1)
    source_stack = (
        '    "color_grading": "escape_default",\n'
        '    "color_source_stack": [\n'
        f"      {row}\n"
        "    ],\n"
    )
    return state_text.replace('    "color_grading": "escape_default",\n', source_stack)


def _with_interior_strength(state_text: str, value: float) -> str:
    return re.sub(
        r'("color_smooth_escape_interior_strength"\s*:\s*)[-+0-9.eE]+',
        rf"\g<1>{value:.6f}",
        state_text,
        count=1,
    )


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


def _capture_loaded_state(exe_path: Path, state_path: Path, out_dir: Path) -> dict[str, object]:
    result = subprocess.run(
        [
            str(exe_path),
            "--load-state-json",
            str(state_path),
            "--capture-diagnostic",
            "--out-dir",
            str(out_dir),
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
    state_text = _with_source_stack(state_path.read_text(encoding="utf-8"), "smooth_escape")
    assert '"color_smooth_escape_interior_strength": 0.2' in state_text
    state_path = tmp_path / "multibrot_source_stack_seed" / "state.json"
    _write_state_text(state_path, state_text)

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


@pytest.mark.parametrize("source_signal", ["smooth_escape", "escape_magnitude"])
def test_smooth_escape_interior_strength_has_visible_source_stack_effect(tmp_path: Path, source_signal: str) -> None:
    if sys.platform != "win32":
        pytest.skip("published viewer runtime capture is Windows-only")

    inventory = _load_inventory_module()
    exe_path = active_runtime_exe()
    seed_capture = _capture_default_escape_time_frame(exe_path, tmp_path / f"multibrot_{source_signal}_metric_seed", "multibrot")
    state_text = _with_source_stack(seed_capture["state_path"].read_text(encoding="utf-8"), source_signal)

    low_state_path = tmp_path / f"{source_signal}_interior_strength_low" / "state.json"
    high_state_path = tmp_path / f"{source_signal}_interior_strength_high" / "state.json"
    _write_state_text(low_state_path, _with_interior_strength(state_text, 0.0))
    _write_state_text(high_state_path, _with_interior_strength(state_text, 1.0))

    low_capture = _capture_loaded_state(exe_path, low_state_path, tmp_path / f"{source_signal}_interior_strength_low_capture")
    high_capture = _capture_loaded_state(exe_path, high_state_path, tmp_path / f"{source_signal}_interior_strength_high_capture")
    low_frame = inventory.read_bmp_rgb(low_capture["frame_path"])
    high_frame = inventory.read_bmp_rgb(high_capture["frame_path"])
    delta = _frame_delta_metrics(low_frame["pixels"], high_frame["pixels"])
    low_metrics = inventory.compute_frame_metrics(low_frame)
    high_metrics = inventory.compute_frame_metrics(high_frame)

    assert delta["changed_pixel_fraction"] > 0.25, {
        "delta": delta,
        "low": low_metrics,
        "high": high_metrics,
    }
    assert delta["avg_abs_rgb_delta_per_channel"] > 20.0, {
        "delta": delta,
        "low": low_metrics,
        "high": high_metrics,
    }
    assert low_metrics["black_pixel_fraction"] > 0.25
    assert high_metrics["black_pixel_fraction"] < 0.05
