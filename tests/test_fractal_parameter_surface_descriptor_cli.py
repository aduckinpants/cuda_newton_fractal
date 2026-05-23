"""Runtime regression tests for the parameter surface descriptor export."""
from __future__ import annotations

import json
import subprocess
import sys

import pytest

from tests.runtime_harness import active_runtime_exe


def _run_descriptor_stdout() -> dict:
    exe = active_runtime_exe()
    result = subprocess.run(
        [str(exe), "--describe-parameter-surface"],
        capture_output=True,
        text=True,
        timeout=10,
    )
    assert result.returncode == 0, f"stderr: {result.stderr}"
    return json.loads(result.stdout)


def _find_lane(descriptor: dict, fractal_id: str) -> dict:
    return next(lane for lane in descriptor["lanes"] if lane["fractal_id"] == fractal_id)


def _find_control(descriptor: dict, fractal_id: str, control_id: str) -> dict | None:
    lane = _find_lane(descriptor, fractal_id)
    return next((control for control in lane["controls"] if control["control_id"] == control_id), None)


def test_parameter_surface_descriptor_cli_owner_controls() -> None:
    if sys.platform != "win32":
        pytest.skip("Windows-only")

    descriptor = _run_descriptor_stdout()
    assert descriptor["version"] == 1
    assert descriptor["fractal_count"] >= 40
    assert descriptor["visible_family_control_cells"] >= 200

    expected = [
        ("burning_ship", "burning_ship_fold_mix", "fractal.params.burning_ship_fold_mix"),
        ("celtic_mandelbrot", "celtic_abs_mix", "fractal.params.celtic_abs_mix"),
        ("perpendicular_burning_ship", "perpendicular_fold_mix", "fractal.params.perpendicular_fold_mix"),
        ("spider", "spider_feedback", "fractal.params.spider_feedback"),
        ("collatz", "collatz_transition_strength", "fractal.params.collatz_transition_strength"),
    ]
    for lane, control_id, binding_path in expected:
        control = _find_control(descriptor, lane, control_id)
        assert control is not None
        assert control["binding_path"] == binding_path
        assert control["runtime_binding_kind"] == "float"
        assert control["binding_resolves"] is True
        assert control["has_validation_range"] is True
        assert control["animatable"] is True
        assert control["state_io_key"] == control_id
        assert _find_control(descriptor, "explaino_all", control_id) is None

    common_expected = [
        ("explaino_warp_strength", "fractal.params.explaino_warp_strength"),
        ("explaino_damping", "fractal.params.explaino_damping"),
    ]
    for control_id, binding_path in common_expected:
        control = _find_control(descriptor, "explaino_nova", control_id)
        assert control is not None
        assert control["binding_path"] == binding_path
        assert control["runtime_binding_kind"] == "float"
        assert control["binding_resolves"] is True
        assert control["has_validation_range"] is True
        assert control["animatable"] is True
        assert control["state_io_key"] == control_id


def test_parameter_surface_descriptor_json_file_output(tmp_path) -> None:
    if sys.platform != "win32":
        pytest.skip("Windows-only")
    exe = active_runtime_exe()
    out_path = tmp_path / "parameter_surface.json"

    result = subprocess.run(
        [str(exe), "--describe-parameter-surface-json", str(out_path)],
        capture_output=True,
        text=True,
        timeout=10,
    )
    assert result.returncode == 0, f"stderr: {result.stderr}"
    descriptor = json.loads(out_path.read_text(encoding="utf-8"))
    assert _find_control(descriptor, "phoenix", "phoenix_p_real")["binding_path"] == "fractal.params.phoenix_p_real"
