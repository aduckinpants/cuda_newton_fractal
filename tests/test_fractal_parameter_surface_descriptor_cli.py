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


def _assert_control(
    descriptor: dict,
    fractal_id: str,
    control_id: str,
    *,
    binding_path: str,
    runtime_kind: str,
    state_key: str,
    visibility_surface_id: str = "default",
    default_visible: bool = True,
    animatable: bool = True,
    absent_from_explaino_all: bool = True,
) -> dict:
    control = _find_control(descriptor, fractal_id, control_id)
    assert control is not None, (fractal_id, control_id)
    assert control["binding_path"] == binding_path
    assert control["runtime_binding_kind"] == runtime_kind
    assert control["binding_resolves"] is True
    assert control["has_validation_range"] is True
    assert control["animatable"] is animatable
    assert control["state_io_key"] == state_key
    assert control["visibility_surface_id"] == visibility_surface_id
    assert control["default_visible"] is default_visible
    if absent_from_explaino_all and fractal_id != "explaino_all":
        assert _find_control(descriptor, "explaino_all", control_id) is None
    return control


def test_parameter_surface_descriptor_cli_owner_controls() -> None:
    if sys.platform != "win32":
        pytest.skip("Windows-only")

    descriptor = _run_descriptor_stdout()
    assert descriptor["version"] == 1
    assert descriptor["fractal_count"] >= 40
    assert descriptor["surface_count"] > descriptor["fractal_count"]
    assert descriptor["default_visible_family_control_cells"] >= 200
    assert descriptor["visible_family_control_cells"] > descriptor["default_visible_family_control_cells"]

    expected = [
        ("burning_ship", "burning_ship_fold_mix", "fractal.params.burning_ship_fold_mix"),
        ("celtic_mandelbrot", "celtic_abs_mix", "fractal.params.celtic_abs_mix"),
        ("perpendicular_burning_ship", "perpendicular_fold_mix", "fractal.params.perpendicular_fold_mix"),
        ("spider", "spider_feedback", "fractal.params.spider_feedback"),
        ("collatz", "collatz_transition_strength", "fractal.params.collatz_transition_strength"),
        ("explaino_collatz_direct", "collatz_transition_strength", "fractal.params.collatz_transition_strength"),
    ]
    for lane, control_id, binding_path in expected:
        _assert_control(
            descriptor,
            lane,
            control_id,
            binding_path=binding_path,
            runtime_kind="float",
            state_key=control_id,
        )

    common_expected = [
        ("explaino_warp_strength", "fractal.params.explaino_warp_strength"),
        ("explaino_damping", "fractal.params.explaino_damping"),
    ]
    for control_id, binding_path in common_expected:
        _assert_control(
            descriptor,
            "explaino_nova",
            control_id,
            binding_path=binding_path,
            runtime_kind="float",
            state_key=control_id,
            absent_from_explaino_all=False,
        )


def test_parameter_surface_descriptor_cli_authority_gated_controls() -> None:
    if sys.platform != "win32":
        pytest.skip("Windows-only")

    descriptor = _run_descriptor_stdout()
    for lane in ("newton", "nova", "halley"):
        _assert_control(
            descriptor,
            lane,
            "poly_c0",
            binding_path="fractal.params.poly_coeffs.0",
            runtime_kind="float",
            state_key="poly_coeffs.0",
            visibility_surface_id="poly_custom",
            default_visible=False,
            animatable=False,
        )
        _assert_control(
            descriptor,
            lane,
            "poly_c4",
            binding_path="fractal.params.poly_coeffs.4",
            runtime_kind="float",
            state_key="poly_coeffs.4",
            visibility_surface_id="poly_custom",
            default_visible=False,
            animatable=False,
        )

    for lane in ("explaino", "explaino_all", "explaino_balance_void"):
        _assert_control(
            descriptor,
            lane,
            "explaino_custom_root_count",
            binding_path="fractal.params.explaino_root_count",
            runtime_kind="int",
            state_key="explaino_root_count",
            visibility_surface_id="explaino_roots_custom",
            default_visible=False,
            animatable=False,
            absent_from_explaino_all=False,
        )
        _assert_control(
            descriptor,
            lane,
            "explaino_root_0_x",
            binding_path="fractal.params.explaino_roots.0.x",
            runtime_kind="float",
            state_key="explaino_roots.0.x",
            visibility_surface_id="explaino_roots_custom",
            default_visible=False,
            animatable=False,
            absent_from_explaino_all=False,
        )

    _assert_control(
        descriptor,
        "explaino_julia",
        "explaino_julia_c_real",
        binding_path="fractal.params.explaino_julia_c_real",
        runtime_kind="float",
        state_key="explaino_julia_c_real",
        visibility_surface_id="explaino_julia_custom",
        default_visible=False,
        animatable=True,
    )
    assert _find_control(descriptor, "explaino_julia", "explaino_root_0_x") is None


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
